#include "worker.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>

#include "select_fd_storage.h"
#include "while_true.h"
#include "queue.h"
#include "tree.h"
#include "fileparser.h"
#include "peer.h"
#include "SMTP_connection.h"
#include "end_marker.h"
#include "errors.h"
#include "global.h"

extern worker_config_t g_config; 
extern peer_t *g_logger;
extern char g_log_message[MAX_g_log_message];

// parent + logger peers
#define ADDITIONAL_PEERS_CNT 2

SMTP_connection_t** get_SMTP_connections(const tree_t *host_vs_peer)
{
    tree_node_t **nodes = tree_nodes_to_array(host_vs_peer);
    if (!nodes)
    {
        return NULL;
    }
    
    int i;
    SMTP_connection_t **peers = (SMTP_connection_t**) malloc(sizeof(SMTP_connection_t*) * host_vs_peer->size);
    if (!peers)
    {
        sprintf(g_log_message, "Ошибка выделения памяти: get_SMTP_connections()");
        send_log();
        free(nodes);
        return NULL;
    }

    for (i = 0; i < host_vs_peer->size; i++)
    {
        peers[i] = (SMTP_connection_t*)nodes[i]->value;
    }
    free(nodes);
    return peers;
}

peer_t** get_peers(SMTP_connection_t **conns, size_t size)
{
    peer_t **peers = (peer_t**) malloc(sizeof(peer_t*) * (size+ADDITIONAL_PEERS_CNT)); 
    if (!peers)
    {
        sprintf(g_log_message, "Ошибка выделения памяти: get_peers()");
        send_log();
        return NULL;
    }
    int i = 0;
    for (i = 0; i < size; i++)
    {
        peers[i] = conns[i]->peer;
    }
    return peers;
}

SMTP_connection_t* get_conn_for_sending(tree_t *host_vs_conn_map, 
                                        const string_t *filename_value)
{
    SMTP_connection_t *peer_for_sending = NULL;
    int addr_type;
    char *addr = get_addr(filename_value, &addr_type);
    printf("%s\n", addr);
    if (!addr)
    {
        return NULL;
    }
    tree_node_t *host_vs_peer = tree_search(host_vs_conn_map, addr);
    if (host_vs_peer)
    {
        peer_for_sending = (SMTP_connection_t*) host_vs_peer->value;
    }
    else
    {
        peer_for_sending = SMTP_connection_init(addr, addr_type);
        if (peer_for_sending)
        {
            tree_insert(host_vs_conn_map, addr, peer_for_sending);
            peer_for_sending = ((SMTP_connection_t*)tree_search(host_vs_conn_map, addr)->value);
        }
    }
    free_addr(addr);
    return peer_for_sending;
}

int remove_file(const char *queue_dir, const string_t *filename)
{
    int ret;
    string_t *queue_dir_str = string_init2(queue_dir, strlen(queue_dir));
    string_t *full_filename = concat_with_sep(queue_dir_str, filename, FILENAME_SEP);
    ret = remove(full_filename->data);
    string_clear(queue_dir_str);
    string_clear(full_filename);
    return ret;
}

int process_parent_messages(tree_t *host_vs_conn_map, queue_t *filenames, const char *queue_dir)
{
    string_t *current_filename = (string_t*)queue_peek(filenames);
    SMTP_connection_t *peer_for_sending = NULL;
    SMTP_message_t *message_for_peer = NULL;

    while(current_filename)
    {
        sprintf(g_log_message, "Найдено письмо для отправки: %s", current_filename->data);
        send_log();
        
        peer_for_sending = get_conn_for_sending(host_vs_conn_map, current_filename);
        if (peer_for_sending)
        {
            message_for_peer = parse_message(queue_dir, current_filename);
            if (message_for_peer)
            {
                queue_push_back(peer_for_sending->messages, message_for_peer);
                SMTP_message_clear(message_for_peer);
            }
        }

        remove_file(queue_dir, current_filename);

        queue_pop_front(filenames);
        current_filename = (string_t*)queue_peek(filenames);
    }
    return SUCCESS;
}

int worker_main(const worker_config_t config)
{
    g_config = config;

    int i = 0;
    peer_t *parent = NULL;
    te_client_fsm_event event;

    tree_t *host_vs_conn_map = TREE_INIT(SMTP_connection_t, SMTP_connection_copy, SMTP_connection_clear);
    if (!host_vs_conn_map)
    {
        return MEMORY_ERROR;
    }

    select_fd_storage_t *storage = storage_init();
    if (!storage)
    {
        tree_clear(host_vs_conn_map);
        return MEMORY_ERROR;
    }

    SMTP_connection_t **conns = NULL;
    peer_t **peers = NULL;
    int peers_count = host_vs_conn_map->size;

    parent = peer_init(config.parent_pipe_fd, FDT_PIPE);
    if (!parent)
    {
        tree_clear(host_vs_conn_map);
        storage_clear(storage);
        return MEMORY_ERROR;
    }

    g_logger = peer_init(config.logger_fd, FDT_PIPE);
    if (!g_logger)
    {
        peer_clear(parent);
        tree_clear(host_vs_conn_map);
        storage_clear(storage);
        return MEMORY_ERROR;
    }

    WHILE_TRUE() 
    {        
        conns = get_SMTP_connections(host_vs_conn_map);        
        peers_count = host_vs_conn_map->size;
        peers = get_peers(conns, peers_count);
        if (!peers)
        {
            free(conns);
            continue;
        }
        peers[peers_count] = parent;
        peers[peers_count+1] = g_logger;

        for (i = 0; i < peers_count+ADDITIONAL_PEERS_CNT; i++)
        {
            if (peers[i]->fd == config.logger_fd)
            {
                fill_buffer_in(peers[i]);
            }
            else if (peers[i]->fd != config.parent_pipe_fd)
            {
                event = generate_event(conns[i]);
                conns[i]->state = client_fsm_step(conns[i]->state, event);
                fill_buffer_in(peers[i]);
            }
        }

        select_step(storage, peers, peers_count+ADDITIONAL_PEERS_CNT);

        for (i = 0; i < peers_count+ADDITIONAL_PEERS_CNT; i++)
        {
            if (peers[i]->fd == config.parent_pipe_fd)
            {
                fill_messages_out(peers[i], PARENT_MESSAGE_END_MARKER);
                process_parent_messages(host_vs_conn_map, peers[i]->messages_out, config.queue_dir);
            }
            else if (peers[i]->fd != config.logger_fd)
            {
                fill_messages_out(peers[i], "\r\n");
                event = generate_event(conns[i]);
                conns[i]->state = client_fsm_step(conns[i]->state, event);

                if (conns[i]->state == CLIENT_FSM_ST_DONE)
                {
                    sprintf(g_log_message, "Соединение закрыто: %s (%s)", conns[i]->addr, conns[i]->ip->data);
                    send_log();

                    if (peers[i]->fd > 0)
                        close(peers[i]->fd);
                    tree_delete(host_vs_conn_map, conns[i]->addr);
                }
            }
        }
            
        free(conns);
        free(peers);
    } 

    peer_clear(parent);
    peer_clear(g_logger);
    storage_clear(storage);
    tree_clear(host_vs_conn_map);
   
    printf("Рабочий процесс завершен");

    return SUCCESS;
}


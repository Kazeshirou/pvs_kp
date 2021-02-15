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
    peer_t **peers = (peer_t**) malloc(sizeof(peer_t*) * size+1);
    if (!peers)
    {
        return NULL;
    }
    int i = 0;
    for (i = 0; i < size; i++)
    {
        peers[i] = conns[i]->peer;
    }
    return peers;
}

SMTP_connection_t* get_peer_for_sending(tree_t *host_vs_conn_map, const string_t *filename_value)
{
    SMTP_connection_t *peer_for_sending = NULL;
    char *addr = get_addr(filename_value);
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
        peer_for_sending = SMTP_connection_init(addr);
        tree_insert(host_vs_conn_map, addr, peer_for_sending);
        peer_for_sending = ((SMTP_connection_t*)tree_search(host_vs_conn_map, addr)->value);
    }
    free_addr(addr);
    return peer_for_sending;
}

int process_parent_messages(tree_t *host_vs_conn_map, queue_t *filenames, const char *queue_dir)
{
    string_t *current_filename = (string_t*)queue_peek(filenames);
    SMTP_connection_t *peer_for_sending = NULL;
    SMTP_message_t *message_for_peer = NULL;

    while(current_filename)
    {
        printf("qs=%ld", filenames->size);
        printf("%s\n\n", current_filename->data);
        peer_for_sending = get_peer_for_sending(host_vs_conn_map, current_filename);
        if (peer_for_sending)
        {
            message_for_peer = parse_message(queue_dir, current_filename);
            if (message_for_peer)
            {
                queue_push_back(peer_for_sending->messages, message_for_peer);
                SMTP_message_clear(message_for_peer);
            }
        }

        queue_pop_front(filenames);
        current_filename = (string_t*)queue_peek(filenames);
    }
    return SUCCESS;
}

int worker_main(const worker_config_t config)
{
    int i = 0;
    int parent_pipe = config.parent_pipe_fd;
    peer_t *parent = NULL;
    te_client_fsm_event event;

    tree_t *host_vs_conn_map = TREE_INIT(SMTP_connection_t, NULL, NULL);
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

    parent = peer_init(parent_pipe, FDT_PIPE);
    if (!parent)
    {
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

        select_step(storage, peers, peers_count+1);

        for (i = 0; i < peers_count+1; i++)
        {
            if (peers[i]->fd == parent_pipe)
            {
                fill_messages_out(peers[i], PARENT_MESSAGE_SEP_STR);
                process_parent_messages(host_vs_conn_map, peers[i]->messages_out, config.queue_dir);
            }
            else
            {
                fill_messages_out(peers[i], "\r\n");
                event = generate_event(conns[i]);
                if (event != CLIENT_FSM_EV_NONE)
                    client_fsm_step(conns[i]->state, event);
                fill_buffer_in(peers[i]);
            }
        }
            
        free(conns);
        free(peers);
    } 

    peer_clear(parent);
    storage_clear(storage);
    tree_clear(host_vs_conn_map);
   
    printf("bye!");

    return SUCCESS;
}


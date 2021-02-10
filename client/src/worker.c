#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "select_fd_storage.h"
#include "while_true.h"
#include "worker.h"
#include "queue.h"
#include "tree.h"
#include "fileparser.h"
#include "peer.h"
#include "end_marker.h"


int connect_server(char *addr)
{
    // create socket
    int fd;
    int port;
    if (strcmp(addr, "s1") == 0)
        port = 3425;
    else
        port = 3426;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
        return -1;
    }
  
    // set up addres
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //inet_addr(server->addr);
    server_addr.sin_port = htons(port);
  
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0) {
        perror("connect()");
        return -1;
    }
  
    printf("Connected to %s:%d.\n", addr, port);
  
    return fd;
}

peer_t** get_peers(const tree_t *host_vs_peer)
{
    tree_node_t **nodes = tree_nodes_to_array(host_vs_peer);
    if (!nodes)
        return NULL;
    
    int i;
    peer_t **peers = (peer_t**) malloc(sizeof(peer_t*) * host_vs_peer->size);
    for (i = 0; i < host_vs_peer->size; i++)
    {
        peers[i] = (peer_t*)nodes[i]->value;
    }
    free(nodes);
    return peers;
}

peer_t* get_peer_for_sending(tree_t *host_vs_peer_map, const string_t *filename_value)
{
    peer_t *peer_for_sending = NULL;
    char *addr = get_addr(filename_value);
    tree_node_t *host_vs_peer = tree_search(host_vs_peer_map, addr);
    if (host_vs_peer)
    {
        peer_for_sending = (peer_t*) host_vs_peer->value;
    }
    else
    {
        peer_for_sending = peer_init(connect_server(addr), FDT_SOCKET);
        tree_insert(host_vs_peer_map, addr, peer_for_sending);
        peer_for_sending = ((peer_t*)tree_search(host_vs_peer_map, addr)->value);
    }
    free_addr(addr);
    return peer_for_sending;
}

int process_parent_messages(tree_t *host_vs_peer_map, queue_t *filenames, const char *queue_dir)
{
    string_t *current_filename = (string_t*)queue_peek(filenames);
    peer_t *peer_for_sending = NULL;
    string_t *message_for_peer = NULL;

    while(current_filename)
    {
        printf("qs=%ld", filenames->size);
        printf("%s\n\n", current_filename->data);
        peer_for_sending = get_peer_for_sending(host_vs_peer_map, current_filename);
        message_for_peer = parse_message(queue_dir, current_filename);
        add_message(peer_for_sending, message_for_peer, "123");

        string_clear(message_for_peer);

        queue_pop_front(filenames);
        current_filename = (string_t*)queue_peek(filenames);
    }
    return 0;
}

int process_server_messages(queue_t *server_messages)
{
    string_t *current_message = (string_t*)queue_peek(server_messages);
    while(current_message)
    {
        printf("size=%ld data=%s\n", current_message->size, current_message->data);

        queue_pop_front(server_messages);
        current_message = (string_t*)queue_peek(server_messages);
    }
    return 0;
}

int worker_main(const worker_config_t config)
{
    int i = 0;
    int parent_pipe = config.parent_pipe_fd;
    peer_t *parent = NULL;
    tree_t *host_vs_peer_map = TREE_INIT(peer_t, peer_copy, peer_clear);
    select_fd_storage_t *storage = storage_init();
    peer_t **peers = NULL;
    int peers_count = host_vs_peer_map->size;

    parent = peer_init(parent_pipe, FDT_PIPE);
    tree_insert(host_vs_peer_map, "parent", parent);
    peer_clear(parent);

    WHILE_TRUE() 
    {        
        peers = get_peers(host_vs_peer_map);
        peers_count = host_vs_peer_map->size;
        if (peers_count > 0)
        {
            for (i = 0; i < peers_count; i++)
            {
                fill_buffer_in(peers[i]);
            }

            select_step(storage, peers, peers_count);

            for (i = 0; i < peers_count; i++)
            {
                if (peers[i]->fd == parent_pipe)
                {
                    fill_messages_out(peers[i], PARENT_MESSAGE_SEP_STR);
                    process_parent_messages(host_vs_peer_map, peers[i]->messages_out, config.queue_dir);
                }
                else
                {
                    fill_messages_out(peers[i], "123");
                    process_server_messages(peers[i]->messages_out);
                }
            }
            
            free(peers);
        }

    } 

    storage_clear(storage);
    tree_clear(host_vs_peer_map);
   
    printf("bye!");

    return 0;
}


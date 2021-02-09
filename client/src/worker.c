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


int connect_server(char *addr, int port)
{
    // create socket
    int fd;
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

peer_t** init_peers() 
{
    int fd1 = connect_server("s1", 3425);
    int fd2 = connect_server("s2", 3426);

    peer_t *server1 = peer_init(fd1, FDT_SOCKET);
    peer_t *server2 = peer_init(fd2, FDT_SOCKET);

    peer_t **peers = (peer_t**)malloc(sizeof(peer_t*) * 2);
    peers[0] = server1;
    peers[1] = server2;
    
    return peers;
}

void clear_peers(peer_t **peers, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        close(peers[i]->fd);
        peer_clear(peers[i]);
    }
    free(peers);
}

peer_t** get_peers_from_host_vs_peer(const tree_t *host_vs_peer)
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

int worker_main(int parent_pipe, config_t config) 
{
    //int i;
    peer_t *parent;
    tree_t *host_vs_peer = TREE_INIT(peer_t, peer_copy, peer_clear);
    select_fd_storage_t *storage = storage_init();
    peer_t **peers;

    parent = peer_init(parent_pipe, FDT_PIPE);
    TREE_INSERT(host_vs_peer, "parent", *parent);
    peer_clear(parent);

    WHILE_TRUE() 
    {        
        peers = get_peers_from_host_vs_peer(host_vs_peer);
        if (peers)
        {
            select_step(storage, peers, host_vs_peer->size);
            free(peers);
        }

        // todo 
        // foreach peer
        //    fill messages out
        //    process messages
        // foreach peer
        //   fill buffer in


    } 

    storage_clear(storage);
    tree_clear(host_vs_peer);
   
    printf("bye!");

    return 0;
}


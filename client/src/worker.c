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

struct peer_t** init_peers() 
{
    int fd1 = connect_server("s1", 3425);
    int fd2 = connect_server("s2", 3426);

    struct peer_t *server1 = peer_init(fd1, FDT_SOCKET);
    struct peer_t *server2 = peer_init(fd2, FDT_SOCKET);

    struct peer_t **peers = (struct peer_t**)malloc(sizeof(struct peer_t*) * 2);
    peers[0] = server1;
    peers[1] = server2;
    
    return peers;
}

void clear_peers(struct peer_t **peers, int size)
{
    int i;
    for (i = 0; i < size; i++)
        peer_clear(peers[i]);
    free(peers);
}

void worker_main(int parent_pipe, struct config_t config) 
{
    //int i;
    int peers_count = 2;
    struct peer_t **peers = init_peers();
    struct select_fd_storage *storage = storage_init();

    WHILE_TRUE() 
    {        
        select_step(storage, peers, peers_count);
    } 

    
    clear_peers(peers, peers_count);
    storage_clear(storage);
   
    printf("hello!");
}


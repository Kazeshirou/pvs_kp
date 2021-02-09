#include <stdlib.h>
#include <stdio.h>

#include "select_fd_storage.h"
#include "queue.h"

select_fd_storage_t* storage_init()
{
    fd_set *read_fds = (fd_set*)malloc(sizeof(fd_set));
    fd_set *write_fds = (fd_set*)malloc(sizeof(fd_set));
    fd_set *except_fds = (fd_set*)malloc(sizeof(fd_set));

    select_fd_storage_t *storage = (select_fd_storage_t*) malloc(sizeof(select_fd_storage_t));
    storage->read_fds = read_fds;
    storage->write_fds = write_fds;
    storage->except_fds = except_fds;
    
    return storage;
}

void storage_clear(select_fd_storage_t *storage)
{
    free(storage->read_fds);
    free(storage->write_fds);
    free(storage->except_fds);
    free(storage);
}

void build_storage(peer_t *server, select_fd_storage_t *storage) 
{
    FD_SET(server->fd, storage->read_fds);
    FD_SET(server->fd, storage->except_fds);
  
    // there is smth to send, set up write_fd for server socket
    if (server->buffer_in_size > 0)
        FD_SET(server->fd, storage->write_fds);
}

void zero_storage(select_fd_storage_t *storage)
{
    FD_ZERO(storage->read_fds);
    FD_ZERO(storage->write_fds);
    FD_ZERO(storage->except_fds);
}

int get_max_fd(peer_t **peers, int size)
{
    int max_fd = -1; 
    int i;
    if (size > 0)
    {
        for (i = 0; i < size; i++)
        {
            if (peers[i]->fd > max_fd)
                max_fd = peers[i]->fd;
        }
    }
    return max_fd;
}

int select_step(select_fd_storage_t *storage, peer_t **peers, int peers_count)
{
    int i;
    int max_fd;
    int ret = 0;

    zero_storage(storage);

    for (i = 0; i < peers_count; i++)
        build_storage(peers[i], storage);

    max_fd = get_max_fd(peers, peers_count);
    ret = select(max_fd + 1, storage->read_fds, storage->write_fds, storage->except_fds, NULL);
    if (ret < 0)
    {
        printf("select()");
        return ret;
    }

    for (i = 0; i < peers_count; i++)
    {
        // if we can read from server socket
        if (FD_ISSET(peers[i]->fd, storage->read_fds)) 
        {
            peer_receive(peers[i]);
        }

        // if we can write to server socket
        if (FD_ISSET(peers[i]->fd, storage->write_fds)) 
        {
            peer_send(peers[i]);
        }
 
        // if exception in server socket
        if (FD_ISSET(peers[i]->fd, storage->except_fds)) 
        {
            printf("except_fds for server.\n");
        }
    } 
    return ret;  
}
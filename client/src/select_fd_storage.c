#include "select_fd_storage.h"

#include <stdio.h>
#include <stdlib.h>

#include "errors.h"
#include "global.h"
#include "queue.h"

select_fd_storage_t* storage_init() {
    select_fd_storage_t* storage =
        (select_fd_storage_t*)malloc(sizeof(select_fd_storage_t));
    if (!storage) {
        sprintf(g_log_message, "Ошибка выделения памяти: storage_init()");
        send_log();
        return NULL;
    }

    fd_set* read_fds = (fd_set*)malloc(sizeof(fd_set));
    if (!read_fds) {
        sprintf(g_log_message, "Ошибка выделения памяти: storage_init()");
        send_log();
        free(storage);
        return NULL;
    }

    fd_set* write_fds = (fd_set*)malloc(sizeof(fd_set));
    if (!write_fds) {
        sprintf(g_log_message, "Ошибка выделения памяти: storage_init()");
        send_log();
        free(read_fds);
        free(storage);
        return NULL;
    }

    storage->read_fds  = read_fds;
    storage->write_fds = write_fds;

    return storage;
}

void storage_clear(select_fd_storage_t* storage) {
    free(storage->read_fds);
    free(storage->write_fds);
    free(storage);
}

void build_storage(peer_t *server, select_fd_storage_t *storage) 
{
    if (server != NULL && server->fd > 0)
    {
        FD_SET(server->fd, storage->read_fds);

        // there is smth to send, set up write_fd for server socket
        if (!queue_is_empty(server->messages_in))
            FD_SET(server->fd, storage->write_fds);
    }
}

void zero_storage(select_fd_storage_t* storage) {
    FD_ZERO(storage->read_fds);
    FD_ZERO(storage->write_fds);
}

int get_max_fd(peer_t** peers, int size) {
    int max_fd = -1;
    int i;
    if (size > 0) {
        for (i = 0; i < size; i++) {
            if (peers[i]->fd > max_fd)
                max_fd = peers[i]->fd;
        }
    }
    return max_fd;
}

int select_step(select_fd_storage_t* storage, peer_t** peers, int peers_count) {
    int            i;
    int            max_fd;
    int            ret = 0;
    struct timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    zero_storage(storage);

    for (i = 0; i < peers_count; i++)
        build_storage(peers[i], storage);

    max_fd = get_max_fd(peers, peers_count);
    ret    = select(max_fd + 1, storage->read_fds, storage->write_fds, NULL,
                 &timeout);
    if (ret < 0) {
        perror("select()");
        // sprintf(g_log_message, "select(): %s", strerror(errno));
        // send_log();
        return SELECT_ERROR;
    }

    for (i = 0; i < peers_count; i++) {
        // if we can read from server socket
        if (FD_ISSET(peers[i]->fd, storage->read_fds)) {
            peer_receive(peers[i]);
        }

        // if we can write to server socket
        if (FD_ISSET(peers[i]->fd, storage->write_fds)) {
            peer_send(peers[i]);
        }
    }
    return ret;
}
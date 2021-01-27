#pragma once

#include <stddef.h>
#include <sys/poll.h>

#include "msg.h"

struct poll_fd_storage {
    struct pollfd* fds;
    struct msg*    msgs;
    size_t         size;
    size_t         max_size;
};

struct poll_fd_storage create_poll_fd_storage(const size_t max_size);
int                    recreate_poll_fd_storage(struct poll_fd_storage* storage,
                                                const size_t            new_max_size);
int                    add_poll_fd_to_storage(struct poll_fd_storage* storage,
                                              const struct pollfd     fd);
void                   free_poll_fd_storage(struct poll_fd_storage* storage);
void compress_poll_fd_storage(struct poll_fd_storage* storage);

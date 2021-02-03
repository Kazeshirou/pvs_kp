#pragma once

#include <stddef.h>
#include <sys/poll.h>

#include "msg.h"

typedef struct {
    struct pollfd* fds;
    struct msg*    msgs;
    size_t         size;
    size_t         max_size;
} server_info_t;

server_info_t create_poll_fd_storage(const size_t max_size);
int recreate_poll_fd_storage(server_info_t* storage, const size_t new_max_size);
int add_poll_fd_to_storage(server_info_t* storage, const struct pollfd fd);
void free_poll_fd_storage(server_info_t* storage);
void compress_poll_fd_storage(server_info_t* storage);

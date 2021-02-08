#pragma once

#include <stddef.h>
#include <sys/select.h>

#include "peer.h"

struct select_fd_storage {
    fd_set *read_fds;
    fd_set *write_fds;
    fd_set *except_fds;
};

struct select_fd_storage* storage_init();
void storage_clear(struct select_fd_storage *storage);

int select_step(struct select_fd_storage *storage, struct peer_t **peers, int peers_count);

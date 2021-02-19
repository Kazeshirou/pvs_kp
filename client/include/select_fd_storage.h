#pragma once

#include <stddef.h>
#include <sys/select.h>

#include "peer.h"

typedef struct select_fd_storage__t {
    fd_set *read_fds;
    fd_set *write_fds;
} select_fd_storage_t;

select_fd_storage_t* storage_init();
void storage_clear(select_fd_storage_t *storage);

int select_step(select_fd_storage_t *storage, peer_t **peers, int peers_count);
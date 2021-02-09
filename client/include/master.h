#pragma once

#include "config.h"
#include "peer.h"
#include "tree.h"

typedef struct worker__t
{
    int pid;
    peer_t *peer_write;
} worker_t;

typedef struct worker_time__t
{
    worker_t worker;
    int last_time_working;
} worker_time_t;

typedef struct state__t
{
    tree_t *addr_vs_worker; // who should process addr
    tree_t *sended_files; // already sended file names
    int rr_index; // for round robin alg
} state_t;

void master_main(config_t config);
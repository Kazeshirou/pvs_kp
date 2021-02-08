#pragma once

#include "config.h"
#include "peer.h"

struct worker_t
{
    int pid;
    struct peer_t *peer_write;
};

struct worker_time_t
{
    struct worker_t worker;
    int last_time_working;
};

struct state_t
{
    struct tree_t *addr_vs_worker; // who should process addr
    struct tree_t *sended_files; // already sended file names
    int rr_index; // for round robin alg

};

void master_main(struct config_t config);
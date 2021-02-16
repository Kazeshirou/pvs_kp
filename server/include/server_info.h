#pragma once

#include <stddef.h>
#include <sys/poll.h>

#include "client.h"
#include "custom_errors.h"

typedef struct {
    uint16_t port;
    size_t   backlog_queue_size;
    char     domain[255];
    char     local_maildir[255];
    char     client_maildir[255];
    char     user[256];
} smtp_server_cfg_t;

typedef struct {
    struct pollfd*           fds;
    client_t**               clients;
    size_t                   size;
    size_t                   max_size;
    const smtp_server_cfg_t* cfg;
    int                      pid;
    int                      tid;
    int                      N;
    char                     hostname[256];
} server_info_t;

error_code_t server_info_init(server_info_t*           server,
                              const smtp_server_cfg_t* cfg,
                              const size_t max_size, const int tid);
error_code_t server_info_resize(server_info_t* server,
                                const size_t   new_max_size);
error_code_t server_info_add_client(server_info_t* server, const int fd);
void         server_info_destroy(server_info_t* server);
void         server_info_compress(server_info_t* server);

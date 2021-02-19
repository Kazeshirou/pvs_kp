#pragma once

#include <stddef.h>
#include <sys/poll.h>

#include "client.h"
#include "custom_errors.h"

typedef struct {
    uint16_t     port;
    const char*  address;
    size_t       backlog_queue_size;
    size_t       thread_pool_size;
    const char*  domain;
    const char*  local_maildir;
    const char*  client_maildir;
    const char*  user;
    const char** relay_networks;
    size_t       relay_count;
    size_t       timeout;
} smtp_server_cfg_t;

typedef struct {
    struct pollfd*           fds;
    client_t**               clients;
    size_t                   size;
    size_t                   max_size;
    const smtp_server_cfg_t* cfg;
    int                      pid;
    size_t                   tid;
    size_t                   id;
    int                      N;
    char                     hostname[256];
    size_t                   timeout;
} server_info_t;

error_code_t server_info_init(server_info_t*           server,
                              const smtp_server_cfg_t* cfg,
                              const size_t max_size, const size_t tid,
                              const size_t id);
error_code_t server_info_resize(server_info_t* server,
                                const size_t   new_max_size);
error_code_t server_info_add_client(server_info_t* server, const int fd);
void         server_info_destroy(server_info_t* server);
void         server_info_compress(server_info_t* server);

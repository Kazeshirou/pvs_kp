#pragma once

#include "custom_errors.h"
#include "server_info.h"

typedef struct {
    uint16_t port;
    size_t   backlog_queue_size;
} smtp_server_cfg_t;

void         set_socket_unblock(const int fd);
error_code_t create_server_socket(const int    port,
                                  const size_t backlog_queue_size,
                                  int*         server_fd);
error_code_t process_listener(const int listener_fd, int* new_client_fd);
error_code_t process_poll(server_info_t* server_info);
void         smtp_server(const smtp_server_cfg_t cfg);
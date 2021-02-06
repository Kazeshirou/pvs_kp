#pragma once

#include "custom_errors.h"
#include "poll_fd_storage.h"

typedef struct {
    uint16_t port;
    size_t   backlog_queue_size;
} smtp_server_cfg_t;

void         set_socket_unblock(const int fd);
error_code_t create_server_socket(const int    port,
                                  const size_t backlog_queue_size,
                                  int*         server_fd);
error_code_t process_listener(const int listener_fd, int* new_client_fd);
error_code_t process_client(server_info_t* storage, const size_t i);
error_code_t process_poll_fds(server_info_t* storage);
void         smtp_server(const smtp_server_cfg_t cfg);
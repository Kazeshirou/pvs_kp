#pragma once

#include "poll_fd_storage.h"

int  process_listener(const int listener_fd, server_info_t* storage);
int  process_client(server_info_t* storage, const size_t i);
int  process_poll_fds(server_info_t* storage);
void echo_server(const int listener_fd);
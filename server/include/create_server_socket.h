#pragma once

#include <stddef.h>  // size_t

int create_server_socket(const int port, const size_t connection_queue_size);
#pragma once

#include <netdb.h>

struct addrinfo* get_server_address(const char* server, const char* port);
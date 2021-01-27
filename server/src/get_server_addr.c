#include "get_server_addr.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

struct addrinfo* get_server_address(const char* server, const char* port) {
    struct addrinfo hints;
    memset(&hints, 0x00, sizeof(hints));
    hints.ai_flags    = AI_NUMERICSERV;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct in6_addr serveraddr;
    if (inet_pton(AF_INET, server, &serveraddr) == 1) {
        // IPv4
        hints.ai_family = AF_INET;
        hints.ai_flags |= AI_NUMERICHOST;
    } else if (inet_pton(AF_INET6, server, &serveraddr) == 1) {
        hints.ai_family = AF_INET6;
        hints.ai_flags |= AI_NUMERICHOST;
    }

    int              getaddr_res;
    struct addrinfo* res = NULL;
    if ((getaddr_res = getaddrinfo(server, port, &hints, &res)) != 0) {
        printf("Host not found --> %s\n", gai_strerror(getaddr_res));
        if (getaddr_res == EAI_SYSTEM) {
            perror("getaddrinfo() failed");
        }
        if (res != NULL) {
            freeaddrinfo(res);
        }
        return NULL;
    }
    return res;
}
#include "server_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

error_code_t server_info_init(server_info_t* server, const size_t max_size) {
    server->fds      = NULL;
    server->clients  = NULL;
    server->size     = 0;
    server->max_size = 0;

    server->fds = (struct pollfd*)calloc(max_size, sizeof(struct pollfd));
    if (!server->fds) {
        printf("Creating of server_info.fds failed");
        return CE_ALLOC;
    }

    server->clients = (client_t**)malloc(max_size * sizeof(client_t*));
    if (!server->clients) {
        printf("Creating of server_info.clients failed");
        free(server->fds);
        server->fds = NULL;
        return CE_ALLOC;
    }

    for (size_t i = 0; i < max_size; i++) {
        server->fds[i].fd  = -1;
        server->clients[i] = NULL;
    }

    server->max_size = max_size;
    return CE_SUCCESS;
}

error_code_t server_info_resize(server_info_t* server,
                                const size_t   new_max_size) {
    if (new_max_size <= server->max_size) {
        return CE_SUCCESS;
    }

    struct pollfd* new_buf =
        (struct pollfd*)calloc(new_max_size, sizeof(struct pollfd));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return CE_ALLOC;
    }

    client_t** new_client_buf =
        (client_t**)malloc(new_max_size * sizeof(client_t*));
    if (new_client_buf) {
        printf("new_client_buf calloc failed");
        free(new_buf);
        return CE_ALLOC;
    }

    for (size_t i = server->max_size; i < new_max_size; i++) {
        new_buf[i].fd     = -1;
        new_client_buf[i] = NULL;
    }

    if (server->fds) {
        memcpy(new_buf, server->fds, server->max_size);
        free(server->fds);
    }

    if (server->clients) {
        memcpy(new_client_buf, server->clients, server->max_size);
        free(server->clients);
    }

    server->fds      = new_buf;
    server->clients  = new_client_buf;
    server->max_size = new_max_size;
    return CE_SUCCESS;
}

error_code_t server_info_add_client(server_info_t* server, const int fd) {
    size_t new_size = server->size + 1;
    if (new_size > server->max_size) {
        error_code_t cerr = server_info_resize(server, new_size + 50);
        if (cerr != CE_SUCCESS) {
            return cerr;
        }
    }
    server->clients[server->size] = (client_t*)malloc(sizeof(client_t));
    if (!server->clients[server->size]) {
        return CE_ALLOC;
    }
    if (client_init(server->clients[server->size]) != CE_SUCCESS) {
        free(server->clients[server->size]);
        server->clients[server->size] = NULL;
    }

    server->fds[server->size].fd     = fd;
    server->fds[server->size].events = POLLIN | POLLOUT;
    server->size                     = new_size;
    return CE_SUCCESS;
}

void server_info_destroy(server_info_t* server) {
    for (size_t i = 0; i < server->size; i++) {
        client_destroy(server->clients[i]);
        free(server->clients[i]);
    }
    free(server->fds);
    server->fds = NULL;
    free(server->clients);
    server->clients = NULL;
}


void server_info_compress(server_info_t* server) {
    for (size_t i = 0; i < server->size; i++) {
        if (server->fds[i].fd >= 0) {
            continue;
        }

        client_destroy(server->clients[i]);
        free(server->clients[i]);
        size_t j = i + 1;
        for (; (server->fds[j].fd < 0) && (j < server->size); j++) {
            if (!server->clients[j]) {
                continue;
            }
            client_destroy(server->clients[j]);
            free(server->clients[j]);
            server->clients = NULL;
        }
        if (j == server->size) {
            server->size = i;
            break;
        }
        strncpy((char*)(server->fds + i), (char*)(server->fds + j),
                sizeof(server->fds[0]) * (server->size - j));
        strncpy((char*)(server->clients + i), (char*)(server->clients + j),
                sizeof(server->clients[0]) * (server->size - j));
        server->size -= j - i;
    }
}
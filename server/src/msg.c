#include "msg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

error_code_t msg_init(msg_t* msg, const size_t max_size) {
    msg->text     = NULL;
    msg->size     = 0;
    msg->max_size = 0;

    msg->text = calloc(max_size, sizeof(char));

    if (!msg->text) {
        printf("Creating of msg failed");
        return CE_ALLOC;
    }

    msg->max_size = max_size;
    return CE_SUCCESS;
}

error_code_t msg_resize(msg_t* msg, const size_t new_max_size) {
    if (new_max_size <= msg->max_size) {
        return CE_SUCCESS;
    }

    char* new_buf = calloc(new_max_size, sizeof(char));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return CE_ALLOC;
    }

    if (msg->text) {
        memcpy(new_buf, msg->text, msg->size);
        free(msg->text);
    }

    msg->text     = new_buf;
    msg->max_size = new_max_size;
    return CE_SUCCESS;
}

error_code_t msg_add_text(msg_t* msg, const char* buf, size_t size) {
    size_t       new_size = msg->size + size;
    error_code_t cerr;
    if (new_size > msg->max_size) {
        cerr = msg_resize(msg, new_size + 50);
        if (cerr != CE_SUCCESS) {
            return cerr;
        }
    }

    memcpy(msg->text + msg->size, buf, size);
    msg->size = new_size;
    return CE_SUCCESS;
}

error_code_t msg_rm_text(msg_t* msg, size_t size) {
    if (msg->size <= size) {
        memset(msg->text, 0, msg->max_size);
        msg->size = 0;
        return CE_SUCCESS;
    }

    msg->size -= size;
    strncpy(msg->text, msg->text + size, msg->size);
    return CE_SUCCESS;
}

error_code_t msg_recv_one(msg_t* msg, int fd, int* is_closed) {
    char buf[1000];
    int  recv_res;
    *is_closed = 0;
    for (int i = 0; i < 100; i++) {
        recv_res = recv(fd, buf, sizeof(buf), 0);
        if (recv_res == 0) {
            *is_closed = 1;
            return CE_SUCCESS;
        }

        if (recv_res > 0) {
            msg_add_text(msg, buf, recv_res);
            continue;
        }

        if (errno == EWOULDBLOCK) {
            return CE_SUCCESS;
        }

        perror("Error on recving");
        return CE_COMMON;
    }
    return CE_SUCCESS;
}

error_code_t msg_send_one(msg_t* msg, int fd) {
    int send_res;
    for (int i = 0; (i < 500) && msg->size; i++) {
        send_res = send(fd, msg->text, msg->size, 0);

        if (send_res == 0) {
            return CE_SUCCESS;
        }

        if (send_res > 0) {
            msg_rm_text(msg, send_res);
            continue;
        }

        if (errno == EWOULDBLOCK) {
            return CE_SUCCESS;
        }

        perror("Error on sending");
        return CE_COMMON;
    }
    return CE_SUCCESS;
}

void msg_clean(msg_t* msg) {
    memset(msg->text, 0, msg->max_size);
    msg->size = 0;
}

void msg_destroy(msg_t* msg) {
    free(msg->text);
    msg->text     = NULL;
    msg->size     = 0;
    msg->max_size = 0;
}
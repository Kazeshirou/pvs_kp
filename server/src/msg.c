#include "msg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "end_marker.h"
#include "while_true.h"

struct msg create_msg(const size_t max_size) {
    struct msg msg = {.text = NULL, .size = 0, .max_size = 0};
    msg.text       = calloc(max_size, sizeof(char));

    if (!msg.text) {
        printf("Creating of msg failed");
        return msg;
    }

    msg.max_size = max_size;
    return msg;
}

int recreate_msg(struct msg* msg, const size_t new_max_size) {
    if (new_max_size <= msg->max_size) {
        return 0;
    }

    char* new_buf = calloc(new_max_size, sizeof(char));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return -1;
    }

    if (msg->text) {
        memcpy(new_buf, msg->text, msg->size);
        free(msg->text);
    }

    msg->text     = new_buf;
    msg->max_size = new_max_size;
    return 1;
}

int add_text_to_message(struct msg* msg, const char* buf, size_t size) {
    size_t new_size = msg->size + size;
    if (new_size > msg->max_size) {
        if (recreate_msg(msg, new_size + 50) < 0) {
            return -1;
        }
    }

    memcpy(msg->text + msg->size, buf, size);
    msg->size = new_size;
    return msg->size;
}

void free_msg(struct msg* msg) {
    free(msg->text);
}

struct msg recv_one_message(int fd) {
    struct msg msg = create_msg(50);
    char       recv_buffer[1000];
    size_t     received = 0;
    WHILE_TRUE() {
        int recv_res = recv(fd, &recv_buffer, sizeof(recv_buffer), 0);
        if (recv_res == 0) {
            printf("The connection is closed\n");
            return msg;
        }
        if (recv_res > 0) {
            received += recv_res;
            if (add_text_to_message(&msg, recv_buffer, recv_res) < 0) {
                return msg;
            }
            continue;
        }

        if (errno != EWOULDBLOCK) {
            perror("recv() failed");
            return msg;
        }

        if (received) {
            return msg;
        }
        sleep(0.1);
    }

    return msg;
}


char* get_msg(struct msg* const msg) {
    const size_t end_marker_size = strlen(end_marker);
    if (msg->size < end_marker_size) {
        return NULL;
    }

    int    flag = 0;
    size_t i    = 0;
    for (; i <= msg->size - end_marker_size; i++) {
        if (!memcmp(msg->text + i, end_marker, end_marker_size)) {
            flag = 1;
            break;
        }
    }

    if (!flag) {
        // Нет готового сообщения.
        return NULL;
    }

    size_t msg_size = i + end_marker_size;
    char*  output   = malloc(msg_size + 1);
    if (!output) {
        //  Не удалось выделить буфер под сообщение.
        return NULL;
    }

    memcpy(output, msg->text, msg_size);
    output[msg_size]    = 0;
    size_t new_msg_size = msg->size - msg_size;
    if (msg->size > msg_size) {
        memmove(msg->text, msg->text + msg_size, new_msg_size);
    }
    msg->size = new_msg_size;
    return output;
}
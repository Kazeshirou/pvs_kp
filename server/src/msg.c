#include "msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

error_code_t msg_init(msg_t* msg, const size_t max_size) {
    msg->text     = NULL;
    msg->size     = 0;
    msg->max_size = 0;

    msg->text = calloc(max_size, sizeof(char));

    if (msg->text) {
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

void msg_destroy(msg_t* msg) {
    free(msg->text);
    msg->text     = NULL;
    msg->size     = 0;
    msg->max_size = 0;
}
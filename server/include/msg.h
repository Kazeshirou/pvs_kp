#pragma once

#include <stddef.h>

#include "custom_errors.h"

typedef struct {
    char*  text;
    size_t size;
    size_t max_size;
} msg_t;

error_code_t msg_init(msg_t* msg, const size_t max_size);
error_code_t msg_resize(msg_t* msg, const size_t new_max_size);
error_code_t msg_add_text(msg_t* msg, const char* buf, size_t size);
error_code_t msg_recv_one(msg_t* msg, int fd);
void         msg_destroy(msg_t* msg);
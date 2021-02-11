#pragma once

#include <stddef.h>

#include "custom_errors.h"

typedef struct {
    size_t current_state;
} client_t;

error_code_t client_init(client_t* client);
void         client_destroy(client_t* client);
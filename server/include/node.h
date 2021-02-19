#pragma once

#include <stddef.h>

#include "custom_errors.h"
#include "destructor.h"

typedef struct node__t {
    void*           value;
    size_t          size;
    struct node__t* next;
} node_t;

error_code_t node_init(node_t* node, size_t size);
error_code_t node_init_with_value(node_t* node, const void* value, size_t size);
error_code_t node_set_value(node_t* node, const void* value, size_t size);
error_code_t node_get_value(const node_t* node, void* value, size_t size);
void         node_destroy(node_t* node, destructor_t value_destructor);

#define NODE_INIT(node_ptr, var) \
    node_init_with_value(node_ptr, &var, sizeof(var))
#define NODE_SET_VALUE(node_ptr, var) \
    node_set_value(node_ptr, &var, sizeof(var))
#define NODE_GET_VALUE(node_ptr, var) \
    node_get_value(node_ptr, &var, sizeof(var))

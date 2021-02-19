#include "node.h"

#include <stdlib.h>
#include <string.h>

error_code_t node_init(node_t* node, size_t size) {
    node->value = malloc(size);
    if (!node->value) {
        return CE_ALLOC;
    }

    return CE_SUCCESS;
}

error_code_t node_init_with_value(node_t* node, const void* value,
                                  size_t size) {
    node->value = malloc(size);
    if (!node->value) {
        return CE_ALLOC;
    }

    memcpy(node->value, value, size);
    node->size = size;
    node->next = NULL;

    return CE_SUCCESS;
}

error_code_t node_set_value(node_t* node, const void* value, size_t size) {
    if (!node->value) {
        return CE_NOT_INITED;
    }

    if (size != node->size) {
        return CE_COMMON;
    }

    memcpy(node->value, value, size);

    return CE_SUCCESS;
}

error_code_t node_get_value(const node_t* node, void* value, size_t size) {
    if (!node->value) {
        return CE_NOT_INITED;
    }

    if (size != node->size) {
        return CE_COMMON;
    }

    memcpy(value, node->value, size);

    return CE_SUCCESS;
}

void node_destroy(node_t* node, destructor_t value_destructor) {
    if (value_destructor) {
        value_destructor(node->value);
    }
    if (node->value) {
        free(node->value);
    }
}

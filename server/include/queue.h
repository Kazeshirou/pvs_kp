#pragma once

#include <threads.h>

#include "custom_errors.h"
#include "node.h"

#define QUEUE_DEFAULT_MAX_SIZE 10000

struct queue_t {
    node_t* front;
    node_t* back;
    size_t  size;
    size_t  max_size;
    mtx_t   mtx;
    cnd_t   cnd;
};


error_code_t queue_init(struct queue_t* queue);
error_code_t queue_push_back(struct queue_t* queue, const void* value,
                             const size_t size);
error_code_t queue_try_pop_front(struct queue_t* queue, void* value,
                                 const size_t size);
error_code_t queue_pop_front(struct queue_t* queue, void* value,
                             const size_t size);
void         queue_destroy(struct queue_t* queue, void (*destroy_value)(void*));

#define QUEUE_PUSH_BACK(queue_ptr, var) \
    queue_push_back(queue_ptr, &var, sizeof(var))
#define QUEUE_TRY_POP_FRONT(queue_ptr, var) \
    queue_try_pop_front(queue_ptr, &var, sizeof(var))
#define QUEUE_POP_FRONT(queue_ptr, var) \
    queue_pop_front(queue_ptr, &var, sizeof(var))

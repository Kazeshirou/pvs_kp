#include "queue.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "while_true.h"

error_code_t queue_init(queue_t* queue) {
    queue->front    = NULL;
    queue->back     = NULL;
    queue->size     = 0;
    queue->max_size = QUEUE_DEFAULT_MAX_SIZE;
    if (mtx_init(&queue->mtx, mtx_plain) != thrd_success) {
        return CE_INIT_3RD;
    }
    if (cnd_init(&queue->cnd) != thrd_success) {
        mtx_destroy(&queue->mtx);
        return CE_INIT_3RD;
    }
    return CE_SUCCESS;
}

error_code_t queue_push_back(queue_t* queue, const void* value,
                             const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return CE_LOCK;
    }
    if (queue->size == queue->max_size) {
        mtx_unlock(&queue->mtx);
        return CE_MAX_SIZE;
    }

    node_t* new_node = (node_t*)malloc(sizeof(node_t*));
    if (!new_node) {
        mtx_unlock(&queue->mtx);
        return CE_ALLOC;
    }

    error_code_t err = node_init_with_value(new_node, value, size);
    if (err != CE_SUCCESS) {
        mtx_unlock(&queue->mtx);
        return err;
    }

    if (queue->back) {
        queue->back->next = new_node;
    } else {
        queue->front = new_node;
    }
    queue->back = new_node;
    queue->size++;

    cnd_signal(&queue->cnd);
    mtx_unlock(&queue->mtx);
    return CE_SUCCESS;
}

error_code_t queue_try_pop_front(queue_t* queue, void* value,
                                 const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return CE_LOCK;
    }
    if (!queue->size) {
        mtx_unlock(&queue->mtx);
        return CE_COMMON;
    }

    node_t*      front_node = queue->front;
    error_code_t err        = node_get_value(front_node, value, size);
    if (err != CE_SUCCESS) {
        mtx_unlock(&queue->mtx);
        return err;
    }

    if (--(queue->size)) {
        queue->front = front_node->next;
    } else {
        queue->front = queue->back = NULL;
    }

    node_destroy(front_node, NULL);
    free(front_node);
    mtx_unlock(&queue->mtx);
    return CE_SUCCESS;
}

error_code_t queue_pop_front(queue_t* queue, void* value, const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return CE_LOCK;
    }

    if (!queue->size) {
        cnd_wait(&queue->cnd, &queue->mtx);
        if (!queue->size) {
            mtx_unlock(&queue->mtx);
            return CE_COMMON;
        }
    }

    node_t*      front_node = queue->front;
    error_code_t err        = node_get_value(front_node, value, size);
    if (err != CE_SUCCESS) {
        mtx_unlock(&queue->mtx);
        return err;
    }

    if (--(queue->size)) {
        queue->front = front_node->next;
    } else {
        queue->front = queue->back = NULL;
    }

    node_destroy(front_node, NULL);
    free(front_node);
    mtx_unlock(&queue->mtx);
    return CE_SUCCESS;
}

void queue_destroy(queue_t* queue, destructor_t value_destructor) {
    mtx_lock(&queue->mtx);

    node_t* front_node = queue->front;
    node_t* next_node;
    while (front_node) {
        next_node = front_node->next;
        node_destroy(front_node, value_destructor);
        free(front_node);
        front_node = next_node;
        queue->size--;
    }
    cnd_broadcast(&queue->cnd);
    cnd_destroy(&queue->cnd);
    mtx_unlock(&queue->mtx);
    mtx_destroy(&queue->mtx);
    queue->front = NULL;
    queue->back  = NULL;
    assert(!queue->size);
}
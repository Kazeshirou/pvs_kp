#include "queue.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

int queue_init(struct queue_t* queue) {
    queue->front    = NULL;
    queue->back     = NULL;
    queue->size     = 0;
    queue->max_size = QUEUE_DEFAULT_MAX_SIZE;
    if (mtx_init(&queue->mtx, mtx_plain) == thrd_success) {
        return -3;
    }
    if (cnd_init(&queue->cnd) == thrd_success) {
        mtx_destroy(&queue->mtx);
        return -3;
    }
    return 0;
}

int queue_push_back(struct queue_t* queue, const void* value,
                    const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return -3;
    }
    if (queue->size == queue->max_size) {
        mtx_unlock(&queue->mtx);
        return -2;
    }

    struct node_t* new_node = (struct node_t*)malloc(sizeof(struct node_t*));
    if (!new_node) {
        mtx_unlock(&queue->mtx);
        return -1;
    }

    new_node->value = malloc(size);
    if (!new_node->value) {
        mtx_unlock(&queue->mtx);
        return -1;
    }

    memcpy(new_node->value, value, size);
    new_node->size = size;
    new_node->next = NULL;

    if (queue->back) {
        queue->back->next = new_node;
    } else {
        queue->front = new_node;
    }
    queue->back = new_node;
    queue->size++;

    cnd_signal(&queue->cnd);
    mtx_unlock(&queue->mtx);
    return 0;
}

int queue_try_pop_front(struct queue_t* queue, void* value, const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return -3;
    }
    if (!queue->size) {
        mtx_unlock(&queue->mtx);
        return -1;
    }

    struct node_t* front_node = queue->front;
    if (front_node->size != size) {
        mtx_unlock(&queue->mtx);
        return -2;
    }

    memcpy(value, front_node->value, size);
    free(front_node->value);

    if (--queue->size) {
        queue->front = front_node->next;
    } else {
        queue->front = queue->back = NULL;
    }

    free(front_node);
    mtx_unlock(&queue->mtx);
    return 0;
}

int queue_pop_front(struct queue_t* queue, void* value, const size_t size) {
    if (mtx_lock(&queue->mtx) != thrd_success) {
        return -3;
    }

    while (!queue->size) {
        cnd_wait(&queue->cnd, &queue->mtx);
    }

    struct node_t* front_node = queue->front;
    if (front_node->size != size) {
        mtx_unlock(&queue->mtx);
        return -2;
    }

    memcpy(value, front_node->value, size);
    free(front_node->value);

    if (--queue->size) {
        queue->front = front_node->next;
    } else {
        queue->front = queue->back = NULL;
    }

    free(front_node);
    mtx_unlock(&queue->mtx);
    return 0;
}

void queue_destroy(struct queue_t* queue, void (*destroy_value)(void*)) {
    mtx_lock(&queue->mtx);
    struct node_t* front_node = queue->front;
    struct node_t* next_node;
    while (front_node) {
        next_node = front_node->next;
        if (destroy_value) {
            destroy_value(front_node->value);
        }
        free(front_node->value);
        free(front_node);
        front_node = next_node;
        queue->size--;
    }
    cnd_destroy(&queue->cnd);
    mtx_unlock(&queue->mtx);
    mtx_destroy(&queue->mtx);
    assert(!queue->size);
}
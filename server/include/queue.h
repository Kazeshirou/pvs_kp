#pragma once

#include <stddef.h>
#include <threads.h>

struct node_t {
    void*          value;
    size_t         size;
    struct node_t* next;
};

#define QUEUE_DEFAULT_MAX_SIZE 10000

struct queue_t {
    struct node_t* front;
    struct node_t* back;
    size_t         size;
    size_t         max_size;
    mtx_t          mtx;
    cnd_t          cnd;
};


int  queue_init(struct queue_t* queue);
int  queue_push_back(struct queue_t* queue, const void* value,
                     const size_t size);
int  queue_try_pop_front(struct queue_t* queue, void* value, const size_t size);
int  queue_pop_front(struct queue_t* queue, void* value, const size_t size);
void queue_destroy(struct queue_t* queue, void (*destroy_value)(void*));

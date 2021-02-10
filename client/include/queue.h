#pragma once

#include "copy_constructor.h"
#include "destructor.h"
#include "mstring.h"

typedef struct queue_node__t 
{
    void *value;
    struct queue_node__t *next;
} queue_node_t;

#define QUEUE_DEFAULT_MAX_SIZE 10000

typedef struct queue__t 
{
    queue_node_t *front;
    queue_node_t *back;
    size_t size;
    size_t max_size;
    size_t value_size;
    copy_constructor_t value_copy_constr;
    destructor_t destr;
} queue_t;


queue_t* queue_init(size_t value_size, copy_constructor_t value_copy_constr, destructor_t destr);
void queue_clear(queue_t *queue);

int queue_push_back(queue_t *queue, const void *value);
int queue_push_all(queue_t *queue_dst, queue_t *queue_src);
int queue_pop_front(queue_t* queue);
void* queue_peek(queue_t *queue);
int queue_is_empty(queue_t *queue);

#define QUEUE_INIT(type, value_copy_constr, value_destr) \
    queue_init(sizeof(type), value_copy_constr, value_destr)

#define QUEUE_PUSH_BACK(queue, value) \
    queue_push_back(queue, &value)

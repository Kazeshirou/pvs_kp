#pragma once

struct queue_node_t 
{
    void *value;
    size_t value_size;
    struct queue_node_t *next;
};

#define QUEUE_DEFAULT_MAX_SIZE 10000

struct queue_t 
{
    struct queue_node_t *front;
    struct queue_node_t *back;
    size_t size;
    size_t max_size;
};


struct queue_t* queue_init();
void queue_clear(struct queue_t *queue, void (*value_clear)(void*));

int queue_push_back(struct queue_t *queue, const void *value, const size_t size);
int queue_push_all(struct queue_t *queue_dst, struct queue_t *queue_src);
int queue_pop_front(struct queue_t* queue, void (*value_clear)(void*));
void* queue_peek(struct queue_t *queue);

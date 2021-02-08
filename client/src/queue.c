#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_t* queue_init() 
{
    struct queue_t *queue = (struct queue_t*) malloc(sizeof(struct queue_t));
    if (!queue) 
    {
        return NULL;
    }
    queue->front = NULL;
    queue->back = NULL;
    queue->size = 0;
    queue->max_size = QUEUE_DEFAULT_MAX_SIZE;
    return queue;
}

void queue_clear(struct queue_t* queue, void (*value_clear)(void*)) 
{
    struct queue_node_t* front_node = queue->front;
    struct queue_node_t* next_node;
    while (front_node) 
    {
        next_node = front_node->next;
        if (value_clear) 
        {
            (*value_clear)(front_node->value);
        }
        else
        {
            free(front_node->value);
        }
        free(front_node);
        front_node = next_node;
        queue->size--;
    }
    assert(!queue->size);
}

int queue_push_back(struct queue_t* queue, const void* value, const size_t size) 
{

    if (queue->size == queue->max_size) 
    {
        return -2;
    }

    struct queue_node_t* new_node = (struct queue_node_t*)malloc(sizeof(struct queue_node_t*));
    if (!new_node) 
    {
        return -1;
    }

    new_node->value = malloc(size);
    if (!new_node->value) 
    {
        return -1;
    }

    memcpy(new_node->value, value, size);
    new_node->value_size = size;
    new_node->next = NULL;

    if (queue->back) 
    {
        queue->back->next = new_node;
    } else 
    {
        queue->front = new_node;
    }
    queue->back = new_node;
    queue->size++;

    return 0;
}

int queue_push_all(struct queue_t *queue_dst, struct queue_t *queue_src)
{
    int ret = 0;
    struct queue_node_t *current = queue_src->front;
    while (current != NULL && !ret)
    {
        ret = queue_push_back(queue_dst, current->value, current->value_size);
        current = current->next;
    }
    return ret;
}

int queue_pop_front(struct queue_t* queue, void (*value_clear)(void*)) 
{
    if (!queue->size) 
    {
        return -1;
    }

    struct queue_node_t* front_node = queue->front;

    if (value_clear) 
    {
        (*value_clear)(front_node->value);
    }
    else
    {
        free(front_node->value);
    }

    if (--queue->size) 
    {
        queue->front = front_node->next;
    } else 
    {
        queue->front = queue->back = NULL;
    }

    free(front_node);
    return 0;
}

void* queue_peek(struct queue_t *queue)
{
    void *value = NULL;
    if (queue && queue->size)
    {
        struct queue_node_t *front_node = queue->front;
        value = front_node->value;
    }
    return value;
}

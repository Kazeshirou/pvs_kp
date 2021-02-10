#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

queue_t* queue_init(size_t value_size, copy_constructor_t value_copy_constr, destructor_t destr) 
{
    queue_t *queue = (queue_t*) malloc(sizeof(queue_t));
    if (!queue) 
        return NULL;

    queue->front = NULL;
    queue->back = NULL;
    queue->size = 0;
    queue->max_size = QUEUE_DEFAULT_MAX_SIZE;
    queue->value_size = value_size;
    queue->destr = destr;
    queue->value_copy_constr = value_copy_constr;
    return queue;
}

void queue_clear(queue_t* queue) 
{
    queue_node_t* front_node = queue->front;
    queue_node_t* next_node;
    while (front_node) 
    {
        next_node = front_node->next;
        destruct(front_node->value, queue->destr);
        free(front_node);
        front_node = next_node;
        queue->size--;
    }
    assert(!queue->size);
}

int queue_push_back(queue_t* queue, const void* value) 
{

    if (queue->size == queue->max_size) 
        return -2;

    queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t*));
    if (!new_node) 
        return -1;

    new_node->value = copy(value, queue->value_size, queue->value_copy_constr);

    if (!new_node->value)
    {
        free(new_node);
        return -1;
    }

    new_node->next = NULL;

    if (queue->back) 
        queue->back->next = new_node;
    else 
        queue->front = new_node;

    queue->back = new_node;
    queue->size++;

    return 0;
}

int queue_push_all(queue_t *queue_dst, queue_t *queue_src)
{
    int ret = 0;
    queue_node_t *current = queue_src->front;
    while (current != NULL && !ret)
    {
        ret = queue_push_back(queue_dst, current->value);
        current = current->next;
    }
    return ret;
}

int queue_pop_front(queue_t* queue) 
{
    if (!queue->size) 
        return -2;

    queue_node_t* front_node = queue->front;
    destruct(front_node->value, queue->destr);

    if (--queue->size) 
        queue->front = front_node->next;
    else 
        queue->front = queue->back = NULL;

    free(front_node);
    return 0;
}

void* queue_peek(queue_t *queue)
{
    void *value = NULL;
    if (queue && queue->size)
    {
        queue_node_t *front_node = queue->front;
        value = front_node->value;
    }
    return value;
}

int queue_is_empty(queue_t *queue)
{
    return !(queue && queue->size);
}

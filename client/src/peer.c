#include <stdio.h>
#include <stdlib.h>	// free
#include <string.h>

#include "peer.h"
#include "queue.h"
#include "message.h"

/* private */

struct queue_t* parse_buffer(char *buffer, size_t buffer_size, const char *end_marker, size_t *parsed)
{
    int i = 0, j = 0;
    int message_begin = 0, message_end = 0, message_size = 0;
    struct message_t message;
    struct queue_t *messages = queue_init();
    *parsed = 0;
    int end_marker_len = strlen(end_marker);
    for (i = 0; i < buffer_size; i++)
    {
        if (buffer[i] == end_marker[j])
            j++;
        else
            j = 0;
        // end of message is reached
        if (j == end_marker_len)
        {
            message_end = i - end_marker_len;
            message_size = message_end-message_begin;
            message.data = calloc(1, message_size+1);
            memcpy(message.data, buffer + message_begin, message_size);
            message.size = message_size;
            if (queue_push_back(messages, &messages, sizeof(messages)) != 0)
            {
                printf("error"); //todo
            }
            *parsed += message_size + end_marker_len;
            message_begin = i;
        }
    }
    return messages;
}

int can_add_message_full(struct message_t *message, struct peer_t *peer)
{
    return message->size + peer->buffer_in_size <= MAX_BUFFER_SIZE;
}

void add_data_to_buffer_in(struct peer_t *peer, const char *data, size_t size)
{
    char *dst = peer->buffer_in + peer->buffer_in_size;
    memcpy(dst, data, size);
    peer->buffer_in_size += size;
}

void add_message_internal(struct peer_t *peer, struct message_t *message, size_t size, const char *end_marker)
{
    char *src = message->data + peer->message_in_begin;
    add_data_to_buffer_in(peer, src, size);
    peer->message_in_begin = message->size - size;
    if (peer->message_in_begin == 0)
    {
        add_data_to_buffer_in(peer, end_marker, strlen(end_marker));
    }
}

void add_message_full(struct peer_t *peer, struct message_t *message, const char *end_marker)
{
    add_message_internal(peer, message, message->size, end_marker);
}

void add_message_part(struct peer_t *peer, struct message_t *message, const char *end_marker)
{
    add_message_internal(peer, message, MAX_BUFFER_SIZE-peer->buffer_in_size, end_marker);
}

void reduce_buffer(char *buffer, size_t *buffer_size, size_t reduced)
{
    *buffer_size -= reduced;
    memmove(buffer, buffer + reduced, reduced);
}

void reduce_buffer_in(struct peer_t *peer, size_t sended)
{
    reduce_buffer(peer->buffer_in, &(peer->buffer_in_size), sended);
}

void reduce_buffer_out(struct peer_t *peer, size_t parsed)
{
    reduce_buffer(peer->buffer_out, &(peer->buffer_out_size), parsed);
}


/* public */

struct peer_t* peer_init(int fd, char type)
{
    struct peer_t *peer = (struct peer_t*) malloc(sizeof(struct peer_t));
    peer->fd = fd;
    peer->type = type;

    peer->message_in_begin = 0;
    peer->messages_in = queue_init();
    peer->buffer_in_size = 0;
    
    peer->messages_out = queue_init();
    peer->buffer_out_size = 0;

    return peer;
}

void peer_clear(struct peer_t *peer)
{
    queue_clear(peer->messages_in, NULL);
    queue_clear(peer->messages_out, NULL);
    free(peer);
}

int add_message(struct peer_t *peer, struct message_t *message)
{
    return queue_push_back(peer->messages_out, message, sizeof(struct message_t));
}

int peer_send(struct peer_t *peer) 
{
    int sended;
    sended = send(peer->fd, peer->buffer_in, peer->buffer_in_size, 0);
    if (sended != -1)
    {
        reduce_buffer_in(peer, sended);
    }
    return sended;
}

int peer_receive(struct peer_t *peer) 
{
    size_t received;
    size_t size = MAX_BUFFER_SIZE - peer->buffer_out_size - 1;

    received = recv(peer->fd, peer->buffer_out + peer->buffer_out_size, size, 0);
    if (received != -1)
    {
        peer->buffer_out_size += received;
    }

    return received;
}

void fill_buffer_in(struct peer_t *peer, const char *end_marker)
{
    struct message_t *message = (struct message_t*)queue_peek(peer->messages_in);
    while (can_add_message_full(message, peer))
    {
        add_message_full(peer, message, end_marker);
        queue_pop_front(peer->messages_in, NULL); //todo
        message = (struct message_t*)queue_peek(peer->messages_in);
    }
    add_message_part(peer, message, end_marker);
}

int fill_messages_out(struct peer_t *peer, const char *end_marker)
{
    int ret;
    size_t parsed = 0;
    struct queue_t *new_messages;

    new_messages = parse_buffer(peer->buffer_out, peer->buffer_out_size, end_marker, &parsed);
    ret = queue_push_all(peer->messages_out, new_messages);
    queue_clear(new_messages, NULL); //todo

    reduce_buffer_out(peer, parsed);

    return ret;
}
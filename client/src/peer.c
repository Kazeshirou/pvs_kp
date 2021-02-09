#include <stdio.h>
#include <stdlib.h>	// free
#include <string.h>
#include <unistd.h> //read/write

#include "peer.h"
#include "queue.h"
#include "mstring.h"

/* private */

queue_t* parse_buffer(const char *buffer, size_t buffer_size, const char *end_marker, size_t *parsed)
{
    int i = 0, j = 0;
    int message_begin = 0, message_end = 0, message_size = 0;
    string_t *message;
    queue_t *messages = QUEUE_INIT(string_t, string_copy, string_clear);
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

            message = string_init2(buffer + message_begin, message_size);
            if (QUEUE_PUSH_BACK(messages, *message) != 0)
            {
                printf("error"); //todo
            }
            string_clear(message);
            *parsed += message_size + end_marker_len;
            message_begin = i;
        }
    }
    return messages;
}

int can_add_message_full(const string_t message, const peer_t peer)
{
    return message.size + peer.buffer_in_size <= MAX_BUFFER_SIZE;
}

void add_data_to_buffer_in(peer_t *peer, const char *data, size_t size)
{
    char *dst = peer->buffer_in + peer->buffer_in_size;
    memcpy(dst, data, size);
    peer->buffer_in_size += size;
}

void add_message_internal(peer_t *peer, string_t message, size_t size, const char *end_marker)
{
    char *src = message.data + peer->message_in_begin;
    add_data_to_buffer_in(peer, src, size);
    peer->message_in_begin = message.size - size;
    if (peer->message_in_begin == 0)
    {
        add_data_to_buffer_in(peer, end_marker, strlen(end_marker));
    }
}

void add_message_full(peer_t *peer, string_t message, const char *end_marker)
{
    add_message_internal(peer, message, message.size, end_marker);
}

void add_message_part(peer_t *peer, string_t message, const char *end_marker)
{
    add_message_internal(peer, message, MAX_BUFFER_SIZE-peer->buffer_in_size, end_marker);
}

void reduce_buffer(char *buffer, size_t *buffer_size, size_t reduced)
{
    *buffer_size -= reduced;
    memmove(buffer, buffer + reduced, reduced);
}

void reduce_buffer_in(peer_t *peer, size_t sended)
{
    reduce_buffer(peer->buffer_in, &(peer->buffer_in_size), sended);
}

void reduce_buffer_out(peer_t *peer, size_t parsed)
{
    reduce_buffer(peer->buffer_out, &(peer->buffer_out_size), parsed);
}


/* public */

peer_t* peer_init(int fd, char type)
{
    peer_t *peer = (peer_t*) malloc(sizeof(peer_t));
    peer->fd = fd;
    peer->type = type;

    peer->message_in_begin = 0;
    peer->messages_in = QUEUE_INIT(string_t, string_copy, string_clear);
    peer->buffer_in_size = 0;
    
    peer->messages_out = QUEUE_INIT(string_t, string_copy, string_clear);
    peer->buffer_out_size = 0;

    return peer;
}

void* peer_copy(const void *_other)
{
    peer_t *other = (peer_t*) _other;

    peer_t *peer = (peer_t*) malloc(sizeof(peer_t));
    peer->fd = other->fd;
    peer->type = other->type;

    peer->message_in_begin = other->message_in_begin;
    peer->messages_in = QUEUE_INIT(string_t, string_copy, string_clear);
    queue_push_all(peer->messages_in, other->messages_in);
    peer->buffer_in_size = other->buffer_in_size;
    
    peer->messages_out = QUEUE_INIT(string_t, string_copy, string_clear);
    queue_push_all(peer->messages_out, other->messages_out);
    peer->buffer_out_size = other->buffer_out_size;

    return peer;
}

void peer_clear(void *_peer)
{
    peer_t *peer = (peer_t*) _peer;
    queue_clear(peer->messages_in);
    queue_clear(peer->messages_out);
    free(peer);
}


int add_message(peer_t *peer, string_t message)
{
    return QUEUE_PUSH_BACK(peer->messages_in, message);
}

int peer_send(peer_t *peer) 
{
    int ret = 0;
    ssize_t sended;
    switch (peer->type)
    {
    case FDT_SOCKET:
        sended = send(peer->fd, peer->buffer_in, peer->buffer_in_size, 0);
        break;
    case FDT_PIPE:
        sended = write(peer->fd, peer->buffer_in, peer->buffer_in_size);
        break;    
    default:
        ret = -2;
        break;
    }

    printf("sended=%ld\n", sended);

    if (sended != -1)
        reduce_buffer_in(peer, sended);
    else
        ret = -1;
    return ret;
}

int peer_receive(peer_t *peer) 
{
    int ret = 0;
    ssize_t received;
    size_t size = MAX_BUFFER_SIZE - peer->buffer_out_size;

    switch (peer->type)
    {
    case FDT_SOCKET:
        received = recv(peer->fd, peer->buffer_out + peer->buffer_out_size, size, 0);
        break;
    case FDT_PIPE:
        received = read(peer->fd, peer->buffer_out + peer->buffer_out_size, size);
        break;    
    default:
        ret = -2;
        break;
    }

    printf("received=%ld\n", received);
    
    if (received != -1)
        peer->buffer_out_size += received;
    else
        ret = -1;

    return ret;
}

void fill_buffer_in(peer_t *peer, const char *end_marker)
{
    const string_t *message = (string_t*)queue_peek(peer->messages_in);
    while (message && can_add_message_full(*message, *peer))
    {
        add_message_full(peer, *message, end_marker);
        queue_pop_front(peer->messages_in);
        message = (string_t*)queue_peek(peer->messages_in);
    }
    if (message)
        add_message_part(peer, *message, end_marker);
}

int fill_messages_out(peer_t *peer, const char *end_marker)
{
    if (peer->buffer_out_size == 0)
        return 0;

    int ret;
    size_t parsed = 0;
    queue_t *new_messages;

    new_messages = parse_buffer(peer->buffer_out, peer->buffer_out_size, end_marker, &parsed);
    ret = queue_push_all(peer->messages_out, new_messages);
    queue_clear(new_messages); 

    reduce_buffer_out(peer, parsed);

    return ret;
}
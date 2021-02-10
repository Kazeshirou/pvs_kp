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
            message_size = message_end-message_begin+1;

            message = string_init2(buffer + message_begin, message_size);
            if (QUEUE_PUSH_BACK(messages, *message) != 0)
            {
                printf("error"); //todo
            }
            string_clear(message);
            *parsed = i+1;
            message_begin = i+1;
        }
    }
    return messages;
}

int buffer_in_is_full(const peer_t peer)
{
    return peer.buffer_in_size == MAX_BUFFER_SIZE;
}

void add_data_to_buffer_in(peer_t *peer, const char *data, size_t size)
{
    char *dst = peer->buffer_in + peer->buffer_in_size;
    memcpy(dst, data, size);
    peer->buffer_in_size += size;
}

#define MIN(a,b) (a < b) ? a : b

void reduce_messages_in(peer_t *peer, const string_t *message)
{
    size_t available_to_send = MIN(MAX_BUFFER_SIZE, message->size-peer->message_in_begin);
    char *src = message->data + peer->message_in_begin;
    add_data_to_buffer_in(peer, src, available_to_send);
    peer->message_in_begin += available_to_send;
    // if end of message
    if (peer->message_in_begin == message->size)
    {
        queue_pop_front(peer->messages_in);
        peer->message_in_begin = 0;
    }
}

void reduce_buffer(char *buffer, size_t *buffer_size, size_t reduced)
{
    *buffer_size -= reduced;
    memmove(buffer, buffer + reduced, *buffer_size);
}

#define REDUCE_BUFFER_IN(peer, sended) \
    reduce_buffer(peer->buffer_in, &(peer->buffer_in_size), sended)

#define REDUCE_BUFFER_OUT(peer, parsed) \
    reduce_buffer(peer->buffer_out, &(peer->buffer_out_size), parsed)


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


int add_message(peer_t *peer, const string_t *message, const char *end_marker)
{
    int ret;
    if (end_marker != NULL)
    {
        string_t *end_marker_str = string_init2(end_marker, strlen(end_marker));
        string_t *message_with_end = concat(message, end_marker_str);

        ret = queue_push_back(peer->messages_in, message_with_end);

        string_clear(end_marker_str);
        string_clear(message_with_end);
    }
    else
    {
        ret = queue_push_back(peer->messages_in, message);
    }
    
    return ret;
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

    printf("sended=%ld data=%s\n", sended, peer->buffer_in);

    if (sended != -1)
        REDUCE_BUFFER_IN(peer, sended);
    else
        ret = -1;
    return ret;
}

int peer_receive(peer_t *peer) 
{
    int ret = 0;
    ssize_t received;
    size_t size = MAX_BUFFER_SIZE - peer->buffer_out_size;
    if (size == 0)
        return -3;

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

   // printf("received=%ld fd=%d\n", received, peer->fd);
    
    if (received != -1)
        peer->buffer_out_size += received;
    else
        ret = -1;

    return ret;
}

void fill_buffer_in(peer_t *peer)
{
    const string_t *message = (string_t*)queue_peek(peer->messages_in);
    while (!buffer_in_is_full(*peer) && message)
    {
        reduce_messages_in(peer, message);
        message = (string_t*)queue_peek(peer->messages_in);
    }
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

    REDUCE_BUFFER_OUT(peer, parsed);

    return ret;
}
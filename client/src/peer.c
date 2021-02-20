#include "peer.h"
#include <stdio.h>
#include <stdlib.h>  // free
#include <string.h>
#include <unistd.h>  //read/write

#include "errors.h"
#include "global.h"
#include "mstring.h"
#include "queue.h"

/* private */

queue_t* parse_buffer(const char* buffer, size_t buffer_size,
                      const char* end_marker, size_t* parsed) {
    int       i = 0, j = 0;
    int       message_begin = 0, message_end = 0, message_size = 0;
    string_t* message = NULL;
    *parsed           = 0;
    queue_t* messages = QUEUE_INIT(string_t, string_copy, string_clear);
    if (!messages) {
        return NULL;
    }

    int end_marker_len = strlen(end_marker);
    for (i = 0; i < buffer_size; i++) {
        if (buffer[i] == end_marker[j])
            j++;
        else
            j = 0;
        // end of message is reached
        if (j == end_marker_len) {
            message_end  = i - end_marker_len;
            message_size = message_end - message_begin + 1;

            message = string_init2(buffer + message_begin, message_size);
            queue_push_back(messages, message);
            string_clear(message);
            *parsed       = i + 1;
            message_begin = i + 1;
            j = 0;

            if ((message_begin < buffer_size) && (buffer[message_begin] == '\0'))
            {
                message_begin++;
                (*parsed)++;
            }
        }
    }
    return messages;
}

#define BUFFER_IN_IS_NOT_FULL(peer) peer->buffer_in_size != MAX_BUFFER_SIZE

void add_data_to_buffer_in(peer_t* peer, const char* data, size_t size) {
    char* dst = peer->buffer_in + peer->buffer_in_size;
    memcpy(dst, data, size);
    peer->buffer_in_size += size;
}

#define MIN(a, b) (a < b) ? a : b

void reduce_messages_in(peer_t* peer, const string_t* message) {
    size_t available_to_send =
        MIN(MAX_BUFFER_SIZE, message->size - peer->message_in_begin);
    char* src = message->data + peer->message_in_begin;
    add_data_to_buffer_in(peer, src, available_to_send);
    peer->message_in_begin += available_to_send;
    // if end of message
    if (peer->message_in_begin == message->size) {
        queue_pop_front(peer->messages_in);
        peer->message_in_begin = 0;
    }
}

void reduce_buffer(char* buffer, size_t* buffer_size, size_t reduced) {
    *buffer_size -= reduced;
    memmove(buffer, buffer + reduced, *buffer_size);
}

#define REDUCE_BUFFER_IN(peer, sended) \
    reduce_buffer(peer->buffer_in, &(peer->buffer_in_size), sended)

#define REDUCE_BUFFER_OUT(peer, parsed) \
    reduce_buffer(peer->buffer_out, &(peer->buffer_out_size), parsed)


/* public */

peer_t* peer_init(int fd, char type) {
    peer_t* peer = (peer_t*)malloc(sizeof(peer_t));
    if (!peer) {
        sprintf(g_log_message, "Ошибка выделения памяти: string_init()");
        send_log();
        return NULL;
    }

    peer->fd   = fd;
    peer->type = type;

    peer->message_in_begin = 0;
    peer->messages_in      = QUEUE_INIT(string_t, string_copy, string_clear);
    if (!peer->messages_in) {
        free(peer);
        return NULL;
    }
    peer->buffer_in_size = 0;

    peer->messages_out = QUEUE_INIT(string_t, string_copy, string_clear);
    if (!peer->messages_out) {
        free(peer);
        return NULL;
    }
    peer->buffer_out_size = 0;

    return peer;
}

void* peer_copy(const void* _other) {
    peer_t* other = (peer_t*)_other;

    peer_t* peer = (peer_t*)malloc(sizeof(peer_t));
    if (!peer) {
        sprintf(g_log_message, "Ошибка выделения памяти: string_init()");
        send_log();
        return NULL;
    }
    peer->fd   = other->fd;
    peer->type = other->type;

    peer->message_in_begin = other->message_in_begin;
    peer->messages_in      = QUEUE_INIT(string_t, string_copy, string_clear);
    if (!peer->messages_in) {
        free(peer);
        return NULL;
    }
    if (queue_push_all(peer->messages_in, other->messages_in) != SUCCESS) {
        free(peer->messages_in);
        free(peer);
        return NULL;
    }
    peer->buffer_in_size = other->buffer_in_size;

    peer->messages_out = QUEUE_INIT(string_t, string_copy, string_clear);
    if (!peer->messages_out) {
        free(peer);
        return NULL;
    }
    if (queue_push_all(peer->messages_out, other->messages_out) != SUCCESS) {
        free(peer->messages_in);
        free(peer->messages_out);
        free(peer);
        return NULL;
    }
    peer->buffer_out_size = other->buffer_out_size;

    return peer;
}

void peer_clear(void* _peer) {
    peer_t* peer = (peer_t*)_peer;
    if (peer) {
        queue_clear(peer->messages_in);
        queue_clear(peer->messages_out);
        free(peer);
    }
}


int add_message(peer_t* peer, const string_t* message, const char* end_marker) {
    int       ret;
    string_t* end_marker_str;
    string_t* message_with_end;

    if (end_marker != NULL) {
        end_marker_str = string_init2(end_marker, strlen(end_marker));
        if (end_marker_str) {
            message_with_end = concat(message, end_marker_str);
            if (message_with_end) {
                ret = queue_push_back(peer->messages_in, message_with_end);
                string_clear(message_with_end);
            } else {
                string_clear(end_marker_str);
                ret = MEMORY_ERROR;
            }
            string_clear(end_marker_str);
        } else {
            ret = MEMORY_ERROR;
        }
    } else {
        ret = queue_push_back(peer->messages_in, message);
    }

    return ret;
}

void fill_buffer_in(peer_t *peer)
{
    const string_t *message = (string_t*)queue_peek(peer->messages_in);
    while (BUFFER_IN_IS_NOT_FULL(peer) && message)
    {
        reduce_messages_in(peer, message);
        message = (string_t*)queue_peek(peer->messages_in);
    }
}

int peer_send(peer_t *peer) 
{
    int ret = 0;
    ssize_t sended;

    fill_buffer_in(peer);

    switch (peer->type)
    {
    case FDT_SOCKET:
        sended = send(peer->fd, peer->buffer_in, peer->buffer_in_size, 0);
        break;
    case FDT_PIPE:
        sended = write(peer->fd, peer->buffer_in, peer->buffer_in_size);
        break;    
    default:
        ret = UNKNOWN_FDT;
        break;
    }

    if (sended != -1)
        REDUCE_BUFFER_IN(peer, sended);
    else {
        close(peer->fd);
        peer->fd = -1;
        ret      = SEND_ERROR;
    }
    return ret;
}

int peer_receive(peer_t* peer) {
    int     ret = 0;
    ssize_t received;
    size_t  size = MAX_BUFFER_SIZE - peer->buffer_out_size;
    if (size == 0) {
        sprintf(g_log_message, "Буфер получателя переполнен");
        send_log();
        return RECV_BUFFER_OVERFLOW;
    }

    switch (peer->type) {
        case FDT_SOCKET:
            received = recv(peer->fd, peer->buffer_out + peer->buffer_out_size,
                            size, 0);
            break;
        case FDT_PIPE:
            received =
                read(peer->fd, peer->buffer_out + peer->buffer_out_size, size);
            break;
        default:
            ret = UNKNOWN_FDT;
            break;
    }

    if (received != -1)
        peer->buffer_out_size += received;
    else {
        ret = RECV_ERROR;
        close(peer->fd);
        peer->fd = -1;
    }

    return ret;
}

int fill_messages_out(peer_t *peer, const char *end_marker)
{
    if (peer->buffer_out_size == 0)
        return SUCCESS;

    int      ret;
    size_t   parsed = 0;
    queue_t* new_messages;

    new_messages = parse_buffer(peer->buffer_out, peer->buffer_out_size,
                                end_marker, &parsed);
    if (!new_messages) {
        ret = MEMORY_ERROR;
    } else {
        ret = queue_push_all(peer->messages_out, new_messages);
        queue_clear(new_messages);
    }

    REDUCE_BUFFER_OUT(peer, parsed);

    return ret;
}
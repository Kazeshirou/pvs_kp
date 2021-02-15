#pragma once

#include <netinet/in.h>	// sockaddr_in

#include "queue.h"
#include "mstring.h"

#define MAX_BUFFER_SIZE 1024

#define FDT_SOCKET '0'
#define FDT_PIPE '1'

typedef struct peer__t 
{
    int fd;
    int is_closed;
    char type;

    queue_t *messages_in;
    size_t message_in_begin;
    char buffer_in[MAX_BUFFER_SIZE];
    size_t buffer_in_size;

    queue_t *messages_out;
    char buffer_out[MAX_BUFFER_SIZE];
    size_t buffer_out_size;
} peer_t;

peer_t* peer_init(int fd, char type);
void peer_clear(void *peer);
void* peer_copy(const void *peer);

int add_message(peer_t *peer, const string_t *message, const char *end_marker);

int peer_send(peer_t *peer);
int peer_receive(peer_t *peer);
 
void fill_buffer_in(peer_t *peer);
int fill_messages_out(peer_t *peer, const char *end_marker);

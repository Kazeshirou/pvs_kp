#pragma once

#include <netinet/in.h>	// sockaddr_in

#include "queue.h"
#include "message.h"

#define MAX_BUFFER_SIZE 1024

#define FDT_SOCKET '0'
#define FDT_PIPE '1'

struct peer_t 
{
    int fd;
    char type;

    struct queue_t *messages_in;
    size_t message_in_begin;
    char buffer_in[MAX_BUFFER_SIZE];
    size_t buffer_in_size;

    struct queue_t *messages_out;
    char buffer_out[MAX_BUFFER_SIZE];
    size_t buffer_out_size;
};

struct peer_t* peer_init(int fd, char type);
void peer_clear(struct peer_t *peer);

int add_message(struct peer_t *peer, struct message_t *message);

int peer_send(struct peer_t *peer);
int peer_receive(struct peer_t *peer);
 
void fill_buffer_in(struct peer_t *peer, const char *end_marker);
int fill_messages_out(struct peer_t *peer, const char *end_marker);

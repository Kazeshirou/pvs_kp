#pragma once

#include <stdlib.h>

struct message_t
{
    //char *addr;
    char *data;
    size_t size;
};

inline void message_clear(struct message_t *message)
{
    free(message->data);
    free(message);
}

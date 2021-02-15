#pragma once

#include "mstring.h"

typedef struct SMTP_message__t 
{
    char *from_addr;
    char **recipients_addr;
    size_t recipients_count;
    string_t **msg;
    size_t msg_lines;
    
} SMTP_message_t;

SMTP_message_t* SMTP_message_init();
void* SMTP_message_copy(const void *other);
void SMTP_message_clear(void *msg);
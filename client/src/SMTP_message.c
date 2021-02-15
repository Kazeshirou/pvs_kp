#include "SMTP_message.h"
#include <stdlib.h>

SMTP_message_t* SMTP_message_init()
{
    SMTP_message_t *msg = (SMTP_message_t*) malloc(sizeof(SMTP_message_t));
    if (!msg)
    {
        return NULL;
    }
    msg->msg_lines = 0;
    msg->recipients_count = 0;
    return msg;
}

void SMTP_message_clear(SMTP_message_t *msg)
{
    int i = 0;
    free(msg->from_addr);
    for (i = 0; i < msg->recipients_count; i++)
    {
        free(msg->recipients_addr[i]);
    }
    free(msg->recipients_addr);
    for (i = 0; i < msg->msg_lines; i++)
    {
        free(msg->msg[i]);
    }
    free(msg->msg);
    free(msg);
}
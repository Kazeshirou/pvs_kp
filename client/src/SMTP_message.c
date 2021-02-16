#include "SMTP_message.h"
#include <stdlib.h>
#include <string.h>

#include "global.h"

extern worker_config_t g_config;

SMTP_message_t* SMTP_message_init()
{
    SMTP_message_t *msg = (SMTP_message_t*) malloc(sizeof(SMTP_message_t));
    if (!msg)
    {
        return NULL;
    }
    msg->msg_lines = 0;
    msg->recipients_count = 0;
    msg->max_attempts_time = g_config.max_attempts_time;
    msg->min_interval_between_attempts = g_config.min_interval_between_attempts;
    return msg;
}

void* SMTP_message_copy(const void *vother)
{
    SMTP_message_t *other = (SMTP_message_t *) vother;
    SMTP_message_t *copy = (SMTP_message_t*) malloc(sizeof(SMTP_message_t));
    if (!copy)
    {
        return NULL;
    }

    copy->max_attempts_time = other->max_attempts_time;
    copy->min_interval_between_attempts = other->min_interval_between_attempts;

    copy->recipients_count = other->recipients_count;
    copy->recipients_addr = (char**) malloc(sizeof(char*) * copy->recipients_count);
    int i;
    int len;
    for (i = 0; i < copy->recipients_count; i++)
    {
        len = strlen(other->recipients_addr[i])+1;
        copy->recipients_addr[i] = (char*) malloc(sizeof(char) * len);
        memcpy(copy->recipients_addr[i], other->recipients_addr[i], len);
    }
    copy->msg_lines = other->msg_lines;
    copy->msg = (string_t**) malloc(sizeof(string_t*) * copy->msg_lines);
    for (i = 0; i < copy->msg_lines; i++)
    {
        copy->msg[i] = string_copy(other->msg[i]);
    }
    return copy;
}

void SMTP_message_clear(void *vmsg)
{
    SMTP_message_t* msg = (SMTP_message_t *) vmsg;
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

int is_attempts_time_expired(SMTP_message_t *message)
{
    return time(NULL) - message->attempt_start_time > message->max_attempts_time;
}

int can_start_attempt_now(SMTP_message_t *message)
{
    return time(NULL) - message->last_attempt_time > message->min_interval_between_attempts;
}
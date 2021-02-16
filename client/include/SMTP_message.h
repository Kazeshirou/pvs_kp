#pragma once

#include "mstring.h"
#include "config.h"

typedef struct SMTP_message__t 
{
    char *from_addr;
    char **recipients_addr;
    size_t recipients_count;
    string_t **msg;
    size_t msg_lines;

    time_t attempt_start_time;
    time_t last_attempt_time;

    time_t max_attempts_time;
    time_t min_interval_between_attempts;
    
} SMTP_message_t;

SMTP_message_t* SMTP_message_init();
void* SMTP_message_copy(const void *other);
void SMTP_message_clear(void *msg);

int is_attempts_time_expired(SMTP_message_t *message);
int can_start_attempt_now(SMTP_message_t *message);
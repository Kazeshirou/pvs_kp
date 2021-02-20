#pragma once

#include "mstring.h"
#include "config.h"

/**
 * @brief Структура письма.
**/
typedef struct SMTP_message__t 
{
    char *from_addr; ///< почта отправителя
    char **recipients_addr; ///< почты получателей
    size_t recipients_count; ///< количество получаталей
    string_t **msg; ///< текст письма построчно 
    size_t msg_lines; ///< количество строк в письме

    time_t attempt_start_time; ///< временная метка начала первой попытки отправки
    time_t last_attempt_time; ///< временная метка начала последней попытки отправки

    time_t max_attempts_time; ///< общее время на попытки отправить письмо
    time_t min_interval_between_attempts; ///< минимальное время между попытками
    
} SMTP_message_t;

SMTP_message_t* SMTP_message_init();
void* SMTP_message_copy(const void *other);
void SMTP_message_clear(void *msg);

int is_attempts_time_expired(SMTP_message_t *message);
int can_start_attempt_now(SMTP_message_t *message);
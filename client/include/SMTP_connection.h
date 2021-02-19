#pragma once

#include "queue.h"
#include "peer.h"
#include "client-fsm.h"
#include "config.h"

#include <time.h>

/**
 * @brief Структура соединения на верхнем уровне. 
**/
typedef struct SMTP_connection__t 
{
    char *addr; ///< домен или IP получателя, полученные из названия файла
    string_t *ip; ///< IP получателя 
    int ip_type; ///< тип IP: v4 или v6

    queue_t *messages; ///< еще не отправленные письма
    te_client_fsm_state state; ///< состояние конечного автомата соединения
    te_client_fsm_event last_event; ///< последнее событие
    peer_t *peer; ///< соединение на низком уровне
    size_t current_rcpt; ///< сколько команд RCPT уже было отправлено
    size_t current_msg_line; ///< сколько строк письма уже было отправлено

    int current_connection_num; ///< счетчик (пере)подключений при обрыве соединения
    time_t last_connection_time; ///< временная метка последного (пере)подключения

    int max_connections_count; ///< максимальное количество (пере)подключений при обрыве соединений
    time_t min_interval_between_connections; ///< минимальный интервал между (пере)подключениями

} SMTP_connection_t;

SMTP_connection_t* SMTP_connection_init(const char *addr, int type);
void* SMTP_connection_copy(const void *vother);
void SMTP_connection_clear(void *conn);

te_client_fsm_event generate_event(SMTP_connection_t *conn);
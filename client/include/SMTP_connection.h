#pragma once

#include "queue.h"
#include "peer.h"
#include "client-fsm.h"
#include "config.h"

#include <time.h>

typedef struct SMTP_connection__t 
{
    char *addr;
    queue_t *messages;
    te_client_fsm_state state;
    te_client_fsm_event last_event;
    peer_t *peer;
    size_t current_rcpt;
    size_t current_msg_line;

    int current_connection_num;
    time_t last_connection_time;

    time_t max_connections_count;
    time_t min_interval_between_connections;

    string_t *ip;
    int ip_type;

} SMTP_connection_t;

SMTP_connection_t* SMTP_connection_init(const char *addr, int type);
void* SMTP_connection_copy(const void *vother);
void SMTP_connection_clear(void *conn);

te_client_fsm_event generate_event(SMTP_connection_t *conn);
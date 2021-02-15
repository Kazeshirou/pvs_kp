#pragma once

#include "queue.h"
#include "peer.h"
#include "client-fsm.h"

typedef struct SMTP_connection__t 
{
    queue_t *messages;
    te_client_fsm_state state;
    peer_t *peer;
    size_t current_rcpt;
    size_t current_msg_line;

} SMTP_connection_t;

SMTP_connection_t* SMTP_connection_init(char *addr);
te_client_fsm_event generate_event(SMTP_connection_t *conn);
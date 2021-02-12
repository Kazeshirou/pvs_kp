#pragma once

#include <stddef.h>

#include "client-fsm.h"
#include "custom_errors.h"
#include "msg.h"

typedef struct {
    te_client_state current_state;
    int             need_send;
    msg_t           msg_for_sending;
    int             closed;
} client_t;

error_code_t client_init(client_t* client);
error_code_t client_process_recv(client_t* client, msg_t* msg);
error_code_t client_process_send(client_t* client);
error_code_t client_process_check_timeout(client_t* client);
void         client_destroy(client_t* client);
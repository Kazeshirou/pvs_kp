#pragma once

#include <stddef.h>

#include "client-fsm.h"
#include "custom_errors.h"
#include "mail_writer.h"
#include "msg.h"
#include "smtp_cmd.h"
#include "smtp_definitions.h"

typedef struct {
    te_client_state current_state;
    int             ehlo;
    int             need_send;
    msg_t           greating_info;
    msg_t           msg_for_sending;
    msg_t           from;
    receiver_t      to[SMTP_RCPT_MAX_SIZE];
    size_t          to_count;
    msg_t           msg_text;
    int             closed;
    mail_writer_t   mail_writer;
    size_t          timeout_start_time;
} client_t;

error_code_t client_init(client_t* client, const mail_writer_t* mail_writer);
error_code_t client_process_recv(client_t* client, msg_t* msg);
error_code_t client_process_send(client_t* client);
error_code_t client_process_check_timeout(client_t* client, size_t timeout);
error_code_t client_process_shutdown(client_t* client);
error_code_t client_start_timeout(client_t* client);
error_code_t client_add_greating_info(client_t* client, const char* buf,
                                      size_t size);
error_code_t client_add_mail_from(client_t* client, const char* buf,
                                  size_t size);
error_code_t client_add_rcpt_to(client_t* client, match_info_t* mi);
error_code_t client_add_msg_txt(client_t* client, const char* buf, size_t size);
error_code_t client_data_end_process(client_t* client);
error_code_t client_set_response(client_t* client, const char* buf,
                                 size_t size);
void         client_destroy(client_t* client);
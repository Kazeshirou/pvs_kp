#pragma once

#include <stddef.h>

#include "client-fsm.h"
#include "custom_errors.h"
#include "mail_writer.h"
#include "msg.h"

#define CLIENT_CR_LF                "\r\n"
#define CLIENT_OPENING_MSG          "220" CLIENT_CR_LF
#define CLIENT_SUCCESS_ANSWER       "250 OK" CLIENT_CR_LF
#define CLIENT_DATA_ACCEPTED_ANSWER "354" CLIENT_CR_LF
#define CLIENT_QUIT_SUCCESS_ANSWER  "221 OK" CLIENT_CR_LF

#define MAX_RCPT_TO_COUNT 100

typedef struct {
    te_client_state current_state;
    int             ehlo;
    int             need_send;
    msg_t           greating_info;
    msg_t           msg_for_sending;
    msg_t           from;
    msg_t           to[MAX_RCPT_TO_COUNT];
    size_t          to_count;
    msg_t           msg_text;
    int             closed;
    mail_writer_t   mail_writer;
} client_t;

error_code_t client_init(client_t* client, const mail_writer_t* mail_writer);
error_code_t client_process_recv(client_t* client, msg_t* msg);
error_code_t client_process_send(client_t* client);
error_code_t client_process_check_timeout(client_t* client);
error_code_t client_add_greating_info(client_t* client, const char* buf,
                                      size_t size);
error_code_t client_add_mail_from(client_t* client, const char* buf,
                                  size_t size);
error_code_t client_add_rcpt_to(client_t* client, const char* buf, size_t size);
error_code_t client_add_msg_txt(client_t* client, const char* buf, size_t size);
error_code_t client_data_end_process(client_t* client);
error_code_t client_set_response(client_t* client, const char* buf,
                                 size_t size);
void         client_destroy(client_t* client);
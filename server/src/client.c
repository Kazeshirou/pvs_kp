#include "client.h"

#include "smtp_cmd.h"

#define CLIENT_CR_LF       "\r\n"
#define CLIENT_OPENING_MSG "220" CLIENT_CR_LF

error_code_t client_init(client_t* client) {
    client->current_state = CLIENT_ST_INIT;
    client->need_send     = 0;
    client->closed        = 0;
    if (msg_init(&client->msg_for_sending, 100) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }
    if (msg_add_text(&client->msg_for_sending, CLIENT_OPENING_MSG,
                     sizeof(CLIENT_OPENING_MSG)) != CE_SUCCESS) {
        msg_destroy(&client->msg_for_sending);
        return CE_COMMON;
    }
    client->need_send = 1;
    return CE_SUCCESS;
}

error_code_t client_process_recv(client_t* client, msg_t* msg) {
    match_info_t mi;
    msg->text[msg->size] = 0;
    mi.tested_line       = msg->text;
    te_client_state next_state;
    if (smtp_cmd_check(SMTP_CMD_EHLO, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_EHLO, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_HELO, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_HELO, client, &mi);
    }
    client->current_state = next_state;
    return CE_SUCCESS;
}
error_code_t client_process_send(client_t* client) {
    client->need_send = 0;
    msg_clean(&client->msg_for_sending);
    te_client_state next_state =
        client_step(client->current_state, CLIENT_EV_RESPONSE, client, NULL);
    client->current_state = next_state;
    if (client->current_state == CLIENT_ST_DONE) {
        client->closed = 1;
    }
    return CE_SUCCESS;
}
error_code_t client_process_check_timeout(client_t* client) {
    return CE_SUCCESS;
}

void client_destroy(client_t* client) {
    client->current_state = CLIENT_ST_INIT;
    client->need_send     = 0;
    client->closed        = 1;
    msg_destroy(&client->msg_for_sending);
}
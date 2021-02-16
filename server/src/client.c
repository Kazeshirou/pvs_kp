#include "client.h"

#include <string.h>

#include "smtp_cmd.h"

error_code_t client_init(client_t* client, const mail_writer_t* mail_writer) {
    client->current_state = CLIENT_ST_INIT;
    client->need_send     = 0;
    client->closed        = 0;
    client->ehlo          = 0;
    client->to_count      = 0;
    client->mail_writer   = *mail_writer;

    if (msg_init(&client->greating_info, 100) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }
    if (msg_init(&client->from, 100) != CE_SUCCESS) {
        msg_destroy(&client->greating_info);
        return CE_INIT_3RD;
    }

    if (msg_init(&client->msg_text, 100) != CE_SUCCESS) {
        msg_destroy(&client->greating_info);
        msg_destroy(&client->from);
        return CE_INIT_3RD;
    }

    if (msg_init(&client->msg_for_sending, 100) != CE_SUCCESS) {
        msg_destroy(&client->greating_info);
        msg_destroy(&client->from);
        msg_destroy(&client->msg_text);
        return CE_INIT_3RD;
    }

    if (msg_add_text(&client->msg_for_sending, CLIENT_OPENING_MSG,
                     sizeof(CLIENT_OPENING_MSG)) != CE_SUCCESS) {
        msg_destroy(&client->greating_info);
        msg_destroy(&client->from);
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
    if (client->current_state == CLIENT_ST_EXPECTED_MSG_TEXT_OR_END_MSG) {
        if (smtp_cmd_check(SMTP_CMD_END_DATA, &mi) == CE_SUCCESS) {
            next_state = client_step(client->current_state, CLIENT_EV_END_DATA,
                                     client, &mi);
        } else {
            next_state = client_step(client->current_state, CLIENT_EV_MSG_TEXT,
                                     client, &mi);
        }
        client->current_state = next_state;
        return CE_SUCCESS;
    }

    if (smtp_cmd_check(SMTP_CMD_EHLO, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_EHLO, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_HELO, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_HELO, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_MAIL, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_MAIL, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_RCPT, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_RCPT, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_QUIT, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_QUIT, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_RSET, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_RSET, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_VRFY, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_VERIFY, client, &mi);
    } else if (smtp_cmd_check(SMTP_CMD_DATA, &mi) == CE_SUCCESS) {
        next_state =
            client_step(client->current_state, CLIENT_EV_DATA, client, &mi);
    } else {
        next_state =
            client_step(client->current_state, CLIENT_EV_UNKNOWN, client, &mi);
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

error_code_t client_add_greating_info(client_t* client, const char* buf,
                                      size_t size) {
    if (msg_add_text(&client->greating_info, buf, size) != CE_SUCCESS) {
        return CE_COMMON;
    }

    return CE_SUCCESS;
}

error_code_t client_add_mail_from(client_t* client, const char* buf,
                                  size_t size) {
    msg_clean(&client->from);
    if (msg_add_text(&client->from, buf, size) != CE_SUCCESS) {
        return CE_COMMON;
    }

    return CE_SUCCESS;
}

error_code_t client_add_rcpt_to(client_t* client, match_info_t* mi) {
    if (client->to_count == MAX_RCPT_TO_COUNT) {
        return CE_MAX_SIZE;
    }
    if (msg_init(&client->to[client->to_count].local_part, 50) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }
    if (msg_init(&client->to[client->to_count].domen, 50) != CE_SUCCESS) {
        msg_destroy(&client->to[client->to_count].local_part);
        return CE_INIT_3RD;
    }
    char buf[1000];
    if (smtp_cmd_get_substring(mi, MI_RCPT_TO_POSTMASTER_INDEX, buf,
                               sizeof(buf)) == CE_SUCCESS) {
        msg_add_text(&client->to[client->to_count].local_part, "Postmaster",
                     sizeof("Postmaster"));
        msg_add_text(&client->to[client->to_count].domen,
                     client->mail_writer.domain,
                     strlen(client->mail_writer.domain) + 1);
    } else if (smtp_cmd_get_substring(mi,
                                      MI_RCPT_TO_POSTMASTER_FULL_DOMEN_INDEX,
                                      buf, sizeof(buf)) == CE_SUCCESS) {
        msg_add_text(&client->to[client->to_count].local_part, "Postmaster",
                     sizeof("Postmaster"));
        msg_add_text(&client->to[client->to_count].domen, buf, strlen(buf) + 1);
    } else {
        smtp_cmd_get_substring(mi, MI_RCPT_TO_FORWARD_PATH_LOCAL_PART_INDEX,
                               buf, sizeof(buf));
        msg_add_text(&client->to[client->to_count].local_part, buf,
                     strlen(buf) + 1);
        smtp_cmd_get_substring(mi, MI_RCPT_TO_FORWARD_PATH_DOMAIN_INDEX, buf,
                               sizeof(buf));
        msg_add_text(&client->to[client->to_count].domen, buf, strlen(buf) + 1);
    }
    client->to_count++;
    return CE_SUCCESS;
}

error_code_t client_add_msg_txt(client_t* client, const char* buf,
                                size_t size) {
    if (msg_add_text(&client->msg_text, buf, size) != CE_SUCCESS) {
        return CE_COMMON;
    }

    return CE_SUCCESS;
}

error_code_t client_set_response(client_t* client, const char* buf,
                                 size_t size) {
    msg_clean(&client->msg_for_sending);
    if (msg_add_text(&client->msg_for_sending, buf, size) != CE_SUCCESS) {
        return CE_COMMON;
    }
    client->need_send = 1;
    return CE_SUCCESS;
}

error_code_t client_data_end_process(client_t* client) {
    return write_mail(&client->mail_writer, client->to, client->to_count,
                      &client->msg_text);
}

void client_destroy(client_t* client) {
    for (size_t i = 0; i < client->to_count; i++) {
        msg_destroy(&client->to[i].local_part);
        msg_destroy(&client->to[i].domen);
    }
    client->to_count      = 0;
    client->current_state = CLIENT_ST_INIT;
    client->need_send     = 0;
    client->ehlo          = 0;
    client->closed        = 1;
    msg_destroy(&client->greating_info);
    msg_destroy(&client->from);
    msg_destroy(&client->msg_text);
    msg_destroy(&client->msg_for_sending);
}
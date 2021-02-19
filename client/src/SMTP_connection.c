#include "SMTP_connection.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>  // close


#include "SMTP_command_factory.h"
#include "SMTP_constants.h"
#include "SMTP_message.h"
#include "client-fsm.h"
#include "errors.h"
#include "global.h"

extern worker_config_t g_config;

string_t* get_DNS_record(const char* host, int type) {
    string_t* row_string = NULL;
    int       space_idx  = 0;
    ns_msg    res_msg;
    ns_rr     rr;
    int       res_len = 0;
    u_char    res_buf[4096];
    char      row_buf[4096];

    memset(res_buf, 0, sizeof(res_buf) / sizeof(u_char));
    memset(row_buf, 0, sizeof(row_buf) / sizeof(char));

    res_len = res_query(host, ns_c_in, type, res_buf, sizeof(res_buf));
    if (res_len < 0) {
        return NULL;
    }

    if (ns_initparse(res_buf, res_len, &res_msg) < 0) {
        return NULL;
    }

    res_len = ns_msg_count(res_msg, ns_s_an);
    if (res_len == 0) {
        return NULL;
    }

    if (ns_parserr(&res_msg, ns_s_an, 0, &rr) < 0) {
        return NULL;
    }

    ns_sprintrr(&res_msg, &rr, NULL, NULL, row_buf, sizeof(row_buf));
    for (space_idx = strlen(row_buf);
         space_idx >= 0 && row_buf[space_idx] != ' ' &&
         row_buf[space_idx] != '\t';
         space_idx--)
        ;

    if (space_idx < 0) {
        return NULL;
    }
    space_idx++;
    row_string = string_init2(row_buf + space_idx, strlen(row_buf) - space_idx);
    return row_string;
}

/**
 * Получение IP-адреса почтового сервера
 * @param host Хост
 * @return IP-адрес почтового сервера
 */
string_t* get_IP(const char* host) {
    string_t* ip;
    if (strcmp(host, "localhost") == 0) {
        ip = string_init2("127.0.0.1", 9);
        if (!ip) {
            return NULL;
        }
        return ip;
    }

    string_t* mx = get_DNS_record(host, ns_t_mx);

    if (!mx) {
        return NULL;
    }

    ip = get_DNS_record(mx->data, ns_t_a);
    string_clear(mx);
    if (!ip) {
        return NULL;
    }

    return ip;
}

int connect_server_ipv6(const string_t* ip, int port) {
    int fd;

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (fd < 0) {
        sprintf(g_log_message, "socket(): %s", strerror(errno));
        send_log();
        return -1;
    }

    server_addr.sin6_family = AF_INET6;

    inet_pton(server_addr.sin6_family, ip->data, &server_addr.sin6_addr);
    server_addr.sin6_port = htons(port);

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) 
    {
        sprintf(g_log_message, "connect() to ip %s: %s", strerror(errno), ip->data);
        send_log();
        return -1;
    }

    return fd;
}

int connect_server_ipv4(const string_t* ip, int port) {
    int                fd;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        sprintf(g_log_message, "socket(): %s", strerror(errno));
        send_log();
        return -1;
    }
    server_addr.sin_family = AF_INET;

    inet_pton(server_addr.sin_family, ip->data, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) 
    {
        sprintf(g_log_message, "connect() to ip %s: %s", strerror(errno), ip->data);
        send_log();
        return -1;
    }
    return fd;
}

int connect_server(const string_t* ip, int type) {
    if (ip == NULL)
        return -1;

    int fd;
    int port = 25;

    if (type == AT_IPv4) {
        fd = connect_server_ipv4(ip, port);
    } else if (type == AT_IPv6) {
        fd = connect_server_ipv6(ip, port);
    } else {
        sprintf(g_log_message, "Неизвестный тип адреса %d: connect_server()",
                type);
        send_log();
        return -1;
    }

    if (fd > 0) {
        sprintf(g_log_message, "Успешное подключение: %s:%d", ip->data, port);
        send_log();
    }
    return fd;
}

int can_reconnect_now(SMTP_connection_t *conn)
{
    //printf("%ld %ld\n", time(NULL) - conn->last_connection_time, conn->min_interval_between_connect);
    return (time(NULL) - conn->last_connection_time) > conn->min_interval_between_connect;
}

int has_more_connection_attempts(SMTP_connection_t *conn) 
{
    return conn->max_connect_count > conn->current_connection_num;
}

SMTP_connection_t* SMTP_connection_init(const char* addr, int type) {
    SMTP_connection_t* conn =
        (SMTP_connection_t*)malloc(sizeof(SMTP_connection_t));
    if (!conn) {
        sprintf(g_log_message,
                "Ошибка выделения памяти: SMTP_connection_init()");
        send_log();
        return NULL;
    }
    int fd = -1;

    conn->state = CLIENT_FSM_ST_INIT;
    conn->addr  = (char*)calloc(sizeof(char), strlen(addr) + 1);
    if (!conn->addr) {
        sprintf(g_log_message,
                "Ошибка выделения памяти: SMTP_connection_init()");
        send_log();
        free(conn);
        return NULL;
    }
    memcpy(conn->addr, addr, strlen(addr));
    conn->current_msg_line       = 0;
    conn->current_rcpt           = 0;
    conn->current_connection_num = 0;
    conn->max_connect_count = g_config.max_connect_count;
    conn->min_interval_between_connect = g_config.min_interval_between_connect;

    conn->messages =
        QUEUE_INIT(SMTP_message_t, SMTP_message_copy, SMTP_message_clear);
    if (!conn->messages) {
        free(conn->addr);
        free(conn);
        return NULL;
    }

    if (type == AT_HOST) {
        conn->ip = get_IP(addr);
        type     = AT_IPv4;
    } else {
        conn->ip = string_init2(addr, strlen(addr));
    }
    conn->ip_type = type;

    fd = connect_server(conn->ip, conn->ip_type);

    conn->current_connection_num++;
    conn->last_connection_time = time(NULL);

    conn->peer = peer_init(fd, FDT_SOCKET);
    if (!conn->peer) {
        queue_clear(conn->messages);
        free(conn->ip);
        free(conn->addr);
        free(conn);
        return NULL;
    }

    return conn;
}

void* SMTP_connection_copy(const void* vother) {
    SMTP_connection_t* other = (SMTP_connection_t*)vother;
    SMTP_connection_t* copy =
        (SMTP_connection_t*)malloc(sizeof(SMTP_connection_t));
    if (!copy) {
        sprintf(g_log_message,
                "Ошибка выделения памяти: SMTP_connection_copy()");
        send_log();
        return NULL;
    }

    copy->state = other->state;
    copy->addr  = (char*)calloc(sizeof(char), strlen(other->addr) + 1);
    if (!copy->addr) {
        sprintf(g_log_message,
                "Ошибка выделения памяти: SMTP_connection_copy()");
        send_log();
        free(copy);
        return NULL;
    }
    memcpy(copy->addr, other->addr, strlen(other->addr));
    copy->current_msg_line       = other->current_msg_line;
    copy->current_rcpt           = other->current_rcpt;
    copy->current_connection_num = other->current_connection_num;
    copy->max_connect_count = other->max_connect_count;
    copy->min_interval_between_connect = other->min_interval_between_connect;

    copy->messages =
        QUEUE_INIT(SMTP_message_t, SMTP_message_copy, SMTP_message_clear);
    if (!copy->messages) {
        free(copy->addr);
        free(copy);
        return NULL;
    }
    queue_push_all(copy->messages, other->messages);

    copy->ip      = string_copy(other->ip);
    copy->ip_type = other->ip_type;

    copy->last_connection_time = other->last_connection_time;
    copy->last_event           = other->last_event;

    copy->peer = peer_copy(other->peer);
    if (!copy->peer) {
        queue_clear(copy->messages);
        free(copy->ip);
        free(copy->addr);
        free(copy);
        return NULL;
    }

    return copy;
}

void SMTP_connection_clear(void* vconn) {
    SMTP_connection_t* conn = (SMTP_connection_t*)vconn;
    queue_clear(conn->messages);
    string_clear(conn->ip);
    peer_clear(conn->peer);
    free(conn->addr);
    free(conn);
}

int SMTP_connection_reinit(SMTP_connection_t* conn) {
    conn->state            = CLIENT_FSM_ST_INIT;
    conn->current_msg_line = 0;
    conn->current_rcpt     = 0;
    conn->peer->fd         = connect_server(conn->ip, conn->ip_type);
    conn->current_connection_num++;
    conn->last_connection_time = time(NULL);
    return SUCCESS;
}


int parse_response_code(const string_t* response) {
    int  ans;
    char code[3];

    if (!response || response->size < 3) {
        return -1;
    }

    code[0] = response->data[0];
    code[1] = response->data[1];
    code[2] = response->data[2];
    ans     = atoi(code);

    return ans;
}


te_client_fsm_event generate_event(SMTP_connection_t* conn) {
    string_t*       command  = NULL;
    string_t*       response = NULL;
    SMTP_message_t* message  = NULL;
    int             response_code;

    te_client_fsm_event event = CLIENT_FSM_EV_NONE;
    if (conn->state == CLIENT_FSM_ST_CLOSED_CONNECTION)
        return event;

    if (conn->peer->fd == -1)
    {
        if (!has_more_connection_attempts(conn))
        {
            return CLIENT_FSM_EV_CLOSE_CONNECTION;
        }
        if (can_reconnect_now(conn)) {
            SMTP_connection_reinit(conn);
        }
    }

    switch (conn->state) {
        case CLIENT_FSM_ST_SENDING_HELLO:
            command = HELO_command();
            event   = CLIENT_FSM_EV_HELLO;
            break;
        case CLIENT_FSM_ST_SENDING_EHLO:
            command = EHLO_command();
            event   = CLIENT_FSM_EV_EHLO;
            break;
        case CLIENT_FSM_ST_GET_NEXT_MESSAGE_AND_SENDING_MAIL_OR_QUIT:
            if (!queue_is_empty(conn->messages))
                queue_pop_front(conn->messages);
        case CLIENT_FSM_ST_SENDING_MAIL_OR_QUIT:
            if (queue_is_empty(conn->messages)) {
                command = QUIT_command();
                event   = CLIENT_FSM_EV_QUIT;
            } else {
                message = (SMTP_message_t*)queue_peek(conn->messages);
                if (conn->last_event == CLIENT_FSM_EV_RESPONSE_4XX) {
                    if (!can_start_attempt_now(message)) {
                        event = CLIENT_FSM_EV_NONE;
                        return event;
                    }
                } else {
                    message->attempt_start_time = time(NULL);
                }
                message->last_attempt_time = time(NULL);
                command                = MAILFROM_command(message->from_addr);
                event                  = CLIENT_FSM_EV_MAIL;
                conn->current_rcpt     = 0;
                conn->current_msg_line = 0;
            }
            break;
        case CLIENT_FSM_ST_SENDING_RCPT_OR_DATA:
            message = (SMTP_message_t*)queue_peek(conn->messages);
            if (is_attempts_time_expired(message)) {
                queue_pop_front(conn->messages);
                command = RSET_command();
                event   = CLIENT_FSM_EV_RSET;
            } else {
                if (conn->current_rcpt < message->recipients_count) {
                    command = RCPTTO_command(
                        message->recipients_addr[conn->current_rcpt]);
                    event = CLIENT_FSM_EV_RCPT;
                    conn->current_rcpt++;
                } else {
                    command = DATA_command();
                    event   = CLIENT_FSM_EV_DATA;
                }
            }
            break;
        case CLIENT_FSM_ST_SENDING_MSG_TEXT_OR_END_MSG:
            message = (SMTP_message_t*)queue_peek(conn->messages);
            if (is_attempts_time_expired(message)) {
                queue_pop_front(conn->messages);
                command = RSET_command();
                event   = CLIENT_FSM_EV_RSET;
            } else {
                if (conn->current_msg_line < message->msg_lines) {
                    command = string_copy(message->msg[conn->current_msg_line]);
                    event   = CLIENT_FSM_EV_MSG_TEXT;
                    conn->current_msg_line++;
                } else {
                    command = ENDDATA_command();
                    event   = CLIENT_FSM_EV_END_DATA;
                }
            }
            break;
        case CLIENT_FSM_ST_INIT:
        case CLIENT_FSM_ST_HELLO_SENDED:
        case CLIENT_FSM_ST_EHLO_SENDED:
        case CLIENT_FSM_ST_MAIL_SENDED:
        case CLIENT_FSM_ST_RCPT_SENDED:
        case CLIENT_FSM_ST_DATA_SENDED:
        case CLIENT_FSM_ST_RSET_SENDED:
        case CLIENT_FSM_ST_QUIT_SENDED:
        case CLIENT_FSM_ST_MSG_TEXT_SENDED:
        case CLIENT_FSM_ST_END_DATA_SENDED:
            if (!queue_is_empty(conn->peer->messages_out)) {
                response      = (string_t*)queue_peek(conn->peer->messages_out);
                response_code = parse_response_code(response);

                sprintf(g_log_message, "Получен ответ с кодом %d: %s",
                        response_code, response->data);
                send_log();

                while (!queue_is_empty(conn->peer->messages_out))
                    queue_pop_front(conn->peer->messages_out);

                if (response_code >= 200 && response_code < 300)
                    event = CLIENT_FSM_EV_RESPONSE_2XX;
                else if (response_code >= 300 && response_code < 400)
                    event = CLIENT_FSM_EV_RESPONSE_3XX;
                else if (response_code >= 400 && response_code < 500)
                    event = CLIENT_FSM_EV_RESPONSE_4XX;
                else if (response_code == 502)
                    event = CLIENT_FSM_EV_RESPONSE_502;
                else if (response_code >= 500 && response_code < 600)
                    event = CLIENT_FSM_EV_RESPONSE_5XX;
            }
            break;
        default:
            break;
    }


    if (command) {
        sprintf(g_log_message, "Отправлена команда: %s", command->data);
        send_log();
        add_message(conn->peer, command, NULL);
        free(command);
    }


    conn->last_event = event;
    return event;
}
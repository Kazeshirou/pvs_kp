#include "SMTP_connection.h"

#include <netinet/in.h>
#include <stdio.h>

#include "SMTP_message.h"
#include "SMTP_command_factory.h"
#include "errors.h"
#include "client-fsm.h"


int connect_server(char *addr)
{
    // create socket
    int fd;
    int port;
    if (strcmp(addr, "s1") == 0)
        port = 3425;
    else
        port = 3426;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket()");
        return SOCKET_ERROR;
    }
  
    // set up addres
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //inet_addr(server->addr);
    server_addr.sin_port = htons(port);
  
    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != 0) {
        perror("connect()");
        return CONNECT_ERROR;
    }
  
    printf("Connected to %s:%d.\n", addr, port);
  
    return fd;
}

SMTP_connection_t* SMTP_connection_init(char *addr)
{
    SMTP_connection_t *conn = (SMTP_connection_t*) malloc(sizeof(SMTP_connection_t));
    if (!conn)
    {
        return NULL;
    }

    conn->state = CLIENT_FSM_ST_INIT;
    conn->current_msg_line = 0;
    conn->current_rcpt = 0;

    conn->messages = QUEUE_INIT(SMTP_message_t, SMTP_message_copy, SMTP_message_clear);
    if (!conn->messages)
    {
        free(conn);
        return NULL;
    }

    conn->peer = peer_init(-1, FDT_SOCKET);
    if (!conn->peer)
    {
        queue_clear(conn->messages);
        free(conn);
        return NULL;
    }

    return conn;
}

int parse_response_code(const string_t *response) {
    int ans;
    char code[3];

    if (!response || response->size < 3) {
        return -1;
    }

    code[0] = response->data[0];
    code[1] = response->data[1];
    code[2] = response->data[2];
    ans = atoi(code);

    return ans;
}


te_client_fsm_event generate_event(SMTP_connection_t *conn)
{
    string_t *command = NULL;
    SMTP_message_t *message = NULL;
    te_client_fsm_event event = CLIENT_FSM_EV_NONE;
    int response_code;
    switch (conn->state)
    {
    case CLIENT_FSM_ST_SENDING_HELLO:
        command = HELO_command();
        event = CLIENT_FSM_EV_HELLO;
        break;
    case CLIENT_FSM_ST_SENDING_EHLO:
        command = EHLO_command();
        event = CLIENT_FSM_EV_EHLO;
        break;
    case CLIENT_FSM_ST_SENDING_MAIL_OR_QUIT:
        if (queue_is_empty(conn->messages))
        {
            command = QUIT_command();
            event = CLIENT_FSM_EV_QUIT;
        }
        else
        {
            message = (SMTP_message_t*)queue_peek(conn->messages);
            command = MAILFROM_command(message->from_addr);
            event = CLIENT_FSM_EV_MAIL;
            conn->current_rcpt = 0;
            conn->current_msg_line = 0;
        }        
        break;
    case CLIENT_FSM_ST_SENDING_RCPT_OR_DATA:
        message = (SMTP_message_t*)queue_peek(conn->messages);
        if (conn->current_rcpt == message->recipients_count)
        {
            command = RCPTTO_command(message->recipients_addr[conn->current_rcpt]);
            event = CLIENT_FSM_EV_RCPT;
            conn->current_rcpt++;
        }
        else
        {
            command = DATA_command();
            event = CLIENT_FSM_EV_DATA;
        }        
        break;
    case CLIENT_FSM_ST_SENDING_MSG_TEXT_OR_END_MSG:
        message = (SMTP_message_t*)queue_peek(conn->messages);
        if (conn->current_msg_line == message->msg_lines)
        {
            command = message->msg[conn->current_msg_line];
            event = CLIENT_FSM_EV_MSG_TEXT;
            conn->current_msg_line++;
        }
        else
        {
            command = ENDDATA_command();
            event = CLIENT_FSM_EV_END_DATA;
        }        
        break;
    case CLIENT_FSM_ST_HELLO_SENDED:
    case CLIENT_FSM_ST_EHLO_SENDED:
    case CLIENT_FSM_ST_MAIL_SENDED:
    case CLIENT_FSM_ST_RCPT_SENDED:
    case CLIENT_FSM_ST_DATA_SENDED:
    case CLIENT_FSM_ST_MSG_TEXT_SENDED:
    case CLIENT_FSM_ST_END_DATA_SENDED:
        if (!queue_is_empty(conn->peer->messages_out))
        {
            response_code = parse_response_code((string_t*) queue_peek(conn->peer->messages_out));
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


    if (command)
    {
        add_message(conn->peer, command, "\r\n");
    }

    return event;
}
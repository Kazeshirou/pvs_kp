#include <pcre.h>
#include <stdlib.h>  // atoi
#include <string.h>
#include <threads.h>
#include <unistd.h>  // close

#include "checkoptn.h"

#include "end_program_handler.h"
#include "re_definitions.h"
#include "smtp_cmd.h"
#include "smtp_server.h"
#include "while_true.h"

#define DEFAULT_PORT               64999
#define DEFAULT_BACKLOG_QUEUE_SIZE 100
#define DEFAULT_DOMAIN             "mysmtp.ru"
#define DEFAULT_LOCAL_MAILDIR      "~/.mysmtp/"
#define DEFAULT_CLIENT_MAILDIR     "~/.mysmtp_client/"

volatile sig_atomic_t while_true = 1;

int main(int argc, char* argv[]) {
    //  Обработка аргументов командной строки, сбор конфигурации серевера в
    //  структуру.
    int optct = optionProcess(&serverOptions, argc, argv);
    argc -= optct;
    argv += optct;

    smtp_server_cfg_t cfg = {.port               = DEFAULT_PORT,
                             .backlog_queue_size = DEFAULT_BACKLOG_QUEUE_SIZE,
                             .domain             = {0},
                             .local_maildir      = {0},
                             .client_maildir     = {0},
                             .user               = {0}};


    if (COUNT_OPT(PORT)) {
        cfg.port = OPT_VALUE_PORT;
    }
    if (COUNT_OPT(BACKLOG_QUEUE_SIZE)) {
        cfg.backlog_queue_size = OPT_VALUE_BACKLOG_QUEUE_SIZE;
    }
    if (COUNT_OPT(DOMAIN)) {
        size_t size = (strlen(OPT_ARG(DOMAIN)) + 1 <= sizeof(cfg.domain)) ?
                          strlen(OPT_ARG(DOMAIN)) :
                          sizeof(cfg.domain);
        memcpy(cfg.domain, OPT_ARG(DOMAIN), size);
    } else {
        memcpy(cfg.domain, DEFAULT_DOMAIN, sizeof(DEFAULT_DOMAIN));
    }
    if (COUNT_OPT(LOCAL_MAILDIR)) {
        size_t size =
            (strlen(OPT_ARG(LOCAL_MAILDIR)) + 1 <= sizeof(cfg.local_maildir)) ?
                strlen(OPT_ARG(LOCAL_MAILDIR)) :
                sizeof(cfg.local_maildir);
        memcpy(cfg.local_maildir, OPT_ARG(LOCAL_MAILDIR), size);
    } else {
        memcpy(cfg.local_maildir, DEFAULT_LOCAL_MAILDIR,
               sizeof(DEFAULT_LOCAL_MAILDIR));
    }
    if (COUNT_OPT(CLIENT_MAILDIR)) {
        size_t size = (strlen(OPT_ARG(CLIENT_MAILDIR)) + 1 <=
                       sizeof(cfg.client_maildir)) ?
                          strlen(OPT_ARG(CLIENT_MAILDIR)) :
                          sizeof(cfg.client_maildir);
        memcpy(cfg.client_maildir, OPT_ARG(CLIENT_MAILDIR), size);
    } else {
        memcpy(cfg.client_maildir, DEFAULT_CLIENT_MAILDIR,
               sizeof(DEFAULT_CLIENT_MAILDIR));
    }
    if (COUNT_OPT(USER)) {
        size_t size =
            (strlen(OPT_ARG(USER)) + 1 <= sizeof(cfg.client_maildir)) ?
                strlen(OPT_ARG(USER)) :
                sizeof(cfg.client_maildir);
        memcpy(cfg.user, OPT_ARG(USER), size);
    }

    // Установка обработчика сигнала для gracefull shutdown
    if (set_end_program_handler() < 0) {
        return 0;
    }

    // Компилируем все регулярки.
    if (smtp_cmd_init() != CE_SUCCESS) {
        return 0;
    }

    char         buffstring[100] = {0};
    match_info_t mi;
    mi.tested_line = "ehlo [IPv6:::127.0.0.1]\r\n";
    // printf("%s\n", RE_RCPT);
    error_code_t err = smtp_cmd_check(SMTP_CMD_EHLO, &mi);
    if (err != CE_SUCCESS) {
        // printf("fail\n");
    } else {
        err = smtp_cmd_get_substring(&mi, 21, buffstring, sizeof(buffstring));
    }

    // Запуск сервера.
    smtp_server(cfg);
    smtp_cmd_destroy();
    thrd_exit(0);
    return 0;
}
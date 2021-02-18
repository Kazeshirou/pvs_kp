#include <pcre.h>
#include <stdlib.h>  // atoi
#include <string.h>
#include <threads.h>
#include <unistd.h>  // close

#include "checkoptn.h"

#include "end_program_handler.h"
#include "logger.h"
#include "re_definitions.h"
#include "smtp_cmd.h"
#include "smtp_server.h"
#include "while_true.h"

#define DEFAULT_PORT               64999
#define DEFAULT_ADDRESS            "::"
#define DEFAULT_BACKLOG_QUEUE_SIZE 100
#define DEFAULT_THREAD_POOL_SIZE   4
#define DEFAULT_DOMAIN             "mysmtp.ru"
// для быстрого создания пользователей юзать скрипт
// cheate_user.sh и туда передать первым аргументом эту же папочку, но без слеша
// на конце.
#define DEFAULT_LOCAL_MAILDIR  "/tmp/mysmtp/"
#define DEFAULT_CLIENT_MAILDIR "/tmp/mysmtp_client/"
#define DEFAULT_RELAY_COUNT    0

#define DEFAULT_LOG_PATH "/tmp/mysmtp_log.csv"

volatile sig_atomic_t while_true = 1;

logger_t  logger;
logger_t* logger_ = &logger;

int main(int argc, char* argv[]) {
    //  Обработка аргументов командной строки, сбор конфигурации серевера в
    //  структуру.
    int optct = optionProcess(&serverOptions, argc, argv);
    argc -= optct;
    argv += optct;

    smtp_server_cfg_t cfg = {.port               = DEFAULT_PORT,
                             .address            = DEFAULT_ADDRESS,
                             .backlog_queue_size = DEFAULT_BACKLOG_QUEUE_SIZE,
                             .thread_pool_size   = DEFAULT_THREAD_POOL_SIZE,
                             .client_maildir     = DEFAULT_CLIENT_MAILDIR,
                             .domain             = DEFAULT_DOMAIN,
                             .local_maildir      = DEFAULT_LOCAL_MAILDIR,
                             .user               = "",
                             .relay_networks     = NULL,
                             .relay_count        = 0};


    if (COUNT_OPT(PORT)) {
        cfg.port = OPT_VALUE_PORT;
    }
    if (COUNT_OPT(ADDRESS)) {
        cfg.address = OPT_ARG(ADDRESS);
    }
    if (COUNT_OPT(BACKLOG_QUEUE_SIZE)) {
        cfg.backlog_queue_size = OPT_VALUE_BACKLOG_QUEUE_SIZE;
    }
    if (COUNT_OPT(THREAD_POOL_SIZE)) {
        cfg.backlog_queue_size = OPT_VALUE_THREAD_POOL_SIZE;
    }
    if (COUNT_OPT(DOMAIN)) {
        cfg.domain = OPT_ARG(DOMAIN);
    }
    if (COUNT_OPT(LOCAL_MAILDIR)) {
        cfg.local_maildir = OPT_ARG(LOCAL_MAILDIR);
    }
    if (COUNT_OPT(CLIENT_MAILDIR)) {
        cfg.client_maildir = OPT_ARG(CLIENT_MAILDIR);
    }
    if (COUNT_OPT(USER)) {
        cfg.user = OPT_ARG(USER);
    }
    cfg.relay_count = COUNT_OPT(RELAY);
    if (cfg.relay_count) {
        cfg.relay_networks =
            (const char**)calloc(sizeof(char*), cfg.relay_count);
        if (!cfg.relay_networks) {
            printf("Не удалось выделить память для сетей, для которых разрешён "
                   "релей.\n");
            return 1;
        }
        const char** pp = STACKLST_OPT(RELAY);
        for (size_t i = 0; i < cfg.relay_count; i++) {
            cfg.relay_networks[i] = pp[i];
        }
    }

    // Установка обработчика сигнала для gracefull shutdown
    if (set_end_program_handler() < 0) {
        return 0;
    }

    const char* log_path = DEFAULT_LOG_PATH;
    if (COUNT_OPT(LOG_PATH)) {
        log_path = OPT_ARG(LOG_PATH);
    }
    if (init_logger(log_path) != CE_SUCCESS) {
        printf("Не удалось проинициализировать логер!\n");
        return 1;
    }
    add_output(stderr);
    thrd_t log;
    thrd_create(&log, &logger_thread, NULL);

    // Компилируем все регулярки.
    if (smtp_cmd_init() != CE_SUCCESS) {
        return 0;
    }

    // char         buffstring[100] = {0};
    // match_info_t mi;
    // mi.tested_line = "ehlo [IPv6:::127.0.0.1]\r\n";
    // printf("%s\n", RE_RCPT);
    // error_code_t err = smtp_cmd_check(SMTP_CMD_EHLO, &mi);
    // if (err != CE_SUCCESS) {
    // printf("fail\n");
    // } else {
    //     err = smtp_cmd_get_substring(&mi, 21, buffstring,
    //     sizeof(buffstring));
    // }

    // Запуск сервера.
    smtp_server(cfg);

    // Освобождение ресурсов.
    while_true = 0;
    if (cfg.relay_count) {
        free(cfg.relay_networks);
    }
    smtp_cmd_destroy();
    int log_res;
    thrd_join(log, &log_res);
    destroy_logger();
    return 0;
}
#include <pcre.h>
#include <stdlib.h>  // atoi
#include <string.h>
#include <unistd.h>  // close

#include "checkoptn.h"

#include "end_program_handler.h"
#include "re_definitions.h"
#include "smtp_cmd.h"
#include "smtp_server.h"
#include "while_true.h"

#define DEFAULT_PORT               49001
#define DEFAULT_BACKLOG_QUEUE_SIZE 100

volatile sig_atomic_t while_true = 1;

int main(int argc, char* argv[]) {
    //  Обработка аргументов командной строки, сбор конфигурации серевера в
    //  структуру.
    int optct = optionProcess(&serverOptions, argc, argv);
    argc -= optct;
    argv += optct;

    smtp_server_cfg_t cfg = {
        .port               = DEFAULT_PORT,
        .backlog_queue_size = DEFAULT_BACKLOG_QUEUE_SIZE,
    };

    if (COUNT_OPT(PORT)) {
        cfg.port = OPT_VALUE_PORT;
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
    mi.cmd         = SMTP_CMD_EHLO;
    mi.tested_line = "ehlo [IPv6:::127.0.0.1]\r\n";
    printf("%s\n", RE_EHLO);
    error_code_t err = smtp_cmd_check(&mi);
    if (err != CE_SUCCESS) {
        printf("fail\n");
    } else {
        err = smtp_cmd_get_substring(&mi, 21, buffstring, sizeof(buffstring));
    }

    // Запуск сервера.
    smtp_server(cfg);
    smtp_cmd_destroy();
    return 0;
}
#include <stdlib.h>  // atoi
#include <unistd.h>  // close

#include "checkoptn.h"

#include "end_program_handler.h"
#include "re_definitions.h"
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

    printf(RE_ADDRES_LITERAL "\n");
    // Запуск сервера.
    smtp_server(cfg);
    return 0;
}
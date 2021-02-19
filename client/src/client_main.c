#include "checkoptn.h"

#include "end_program_handler.h"

#include "master.h"
#include "config.h"

#define DEFAULT_INTERVAL	10
#define DEFAULT_TOTAL_TIME	60000
#define DEFAULT_QUEUE_DIR	"/home/olga/pvs_kp/client/queue_dir"
#define DEFAULT_LOG_FILE    "/home/olga/pvs_kp/client/log.log"

int while_true = 1;

int main(int argc, char* argv[]) {
    master_config_t config;


    int optct = optionProcess(&clientOptions, argc, argv);
    argc -= optct;
    argv += optct;

    if (set_end_program_handler() < 0) {
        return 0;
    }

    char *queue_dir = DEFAULT_QUEUE_DIR;
    char *log_file = DEFAULT_LOG_FILE;
    config.queue_dir = queue_dir;
    config.log_file = log_file;
    config.workers_count = 1;
    config.min_interval_working_with_addr = 10000;
    config.max_attempts_time = 10000;
    config.max_connections_count = 5;
    config.min_interval_between_connections = 3;

    /*if (COUNT_OPT(QUEUE_DIR)) {
        queue_dir = OPT_VALUE_QUEUE_DIR;
    }
    printf("queue dir = %s\n", queue_dir);*/
    master_main(config);

    return 0;
}

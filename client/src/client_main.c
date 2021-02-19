#include "checkoptn.h"

#include "end_program_handler.h"

#include "config.h"
#include "global.h"
#include "master.h"

<<<<<<< HEAD
#define DEFAULT_QUEUE_DIR	"/home/olga/pvs_kp/client/queue_dir"
#define DEFAULT_LOG_FILE    "/home/olga/pvs_kp/client/log.log"
#define DEFAULT_WORKERS_COUNT 2
#define DEFAULT_MIN_INTERVAL_WORKING_WITH_ADDR 10
#define DEFAULT_MAX_ATTEMPTS_TIME 30
#define DEFAULT_MIN_INTERVAL_BETWEEN_ATTEMPTS 5
#define DEFAULT_MAX_CONNECT_COUNT 5
#define DEFAULT_MIN_INTERVAL_BETWEEN_CONNECT 1
=======
#define DEFAULT_INTERVAL   10
#define DEFAULT_TOTAL_TIME 60000
#define DEFAULT_QUEUE_DIR  "/home/ntl/projects/pvs_kp/client/queue_dir"
#define DEFAULT_LOG_FILE   "/home/ntl/projects/pvs_kp/client/log.log"
>>>>>>> bebbb582ef5f4e753d5ccfddfb98ffeef370ec6d

int             while_true = 1;
char            g_log_message[MAX_g_log_message + 100];
worker_config_t g_config;
peer_t*         g_logger;

int main(int argc, char* argv[]) {

    int optct = optionProcess(&clientOptions, argc, argv);
    argc -= optct;
    argv += optct;

    if (set_end_program_handler() < 0) {
        return 0;
    }

<<<<<<< HEAD
    master_config_t config = 
    {
        .queue_dir = DEFAULT_QUEUE_DIR,
        .log_file = DEFAULT_LOG_FILE,
        .workers_count = DEFAULT_WORKERS_COUNT,
        .min_interval_working_with_addr = DEFAULT_MIN_INTERVAL_WORKING_WITH_ADDR,
        .max_attempts_time = DEFAULT_MAX_ATTEMPTS_TIME,
        .min_interval_between_attempts = DEFAULT_MIN_INTERVAL_BETWEEN_ATTEMPTS,
        .max_connect_count = DEFAULT_MAX_CONNECT_COUNT,
        .min_interval_between_connect = DEFAULT_MIN_INTERVAL_BETWEEN_CONNECT
    };
=======
    char* queue_dir                         = DEFAULT_QUEUE_DIR;
    char* log_file                          = DEFAULT_LOG_FILE;
    config.queue_dir                        = queue_dir;
    config.log_file                         = log_file;
    config.workers_count                    = 1;
    config.min_interval_working_with_addr   = 10000;
    config.max_attempts_time                = 10000;
    config.max_connections_count            = 5;
    config.min_interval_between_connections = 3;
>>>>>>> bebbb582ef5f4e753d5ccfddfb98ffeef370ec6d

    if (COUNT_OPT(QUEUE_DIR)) 
    {
        config.queue_dir = OPT_ARG(QUEUE_DIR);
    }
    if (COUNT_OPT(LOG_FILE)) 
    {
        config.log_file = OPT_ARG(LOG_FILE);
    }
    if (COUNT_OPT(WORKERS_COUNT)) 
    {
        config.workers_count = OPT_VALUE_WORKERS_COUNT;
    }
    if (COUNT_OPT(MIN_INTERVAL_WORKING_WITH_ADDR)) 
    {
        config.min_interval_working_with_addr = OPT_VALUE_MIN_INTERVAL_WORKING_WITH_ADDR;
    }
    if (COUNT_OPT(MAX_ATTEMPTS_TIME)) 
    {
        config.max_attempts_time = OPT_VALUE_MAX_ATTEMPTS_TIME;
    }
    if (COUNT_OPT(MIN_INTERVAL_BETWEEN_ATTEMPTS)) 
    {
        config.min_interval_between_attempts = OPT_VALUE_MIN_INTERVAL_BETWEEN_ATTEMPTS;
    }
    if (COUNT_OPT(MAX_CONNECT_COUNT)) 
    {
        config.max_connect_count = OPT_VALUE_MAX_CONNECT_COUNT;
    }
    if (COUNT_OPT(MIN_INTERVAL_BETWEEN_CONNECT)) 
    {
        config.min_interval_between_connect = OPT_VALUE_MIN_INTERVAL_BETWEEN_CONNECT;
    }

    
    master_main(config);

    return 0;
}

#pragma once

#include <time.h>

typedef struct master_config__t
{
    char *queue_dir;
    int workers_count;
    int min_interval_working_with_addr;
} master_config_t;

typedef struct worker_config__t
{
    char *queue_dir;
    int parent_pipe_fd;
    int logger_fd;

    time_t max_attempts_time;
    time_t min_interval_between_attempts;

    time_t max_connections_count;
    time_t min_interval_between_connections;
    
} worker_config_t;
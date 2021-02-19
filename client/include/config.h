#pragma once

#include <time.h>

typedef struct master_config__t
{
    const char *queue_dir;
    const char *log_file;
    int workers_count;
    int min_interval_working_with_addr;

    time_t max_attempts_time;
    time_t min_interval_between_attempts;

    int max_connect_count;
    time_t min_interval_between_connect;

    int logger_fd;

} master_config_t;

typedef struct worker_config__t
{
    const char *queue_dir;
    int parent_pipe_fd;
    int logger_fd;

    time_t max_attempts_time;
    time_t min_interval_between_attempts;

    int max_connect_count;
    time_t min_interval_between_connect;
    
} worker_config_t;

typedef struct logger_config__t
{
    const char *log_file;
    int *worker_pipe_fds;
    int workers_count;
        
} logger_config_t;
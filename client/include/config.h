#pragma once

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
    // todo other params
} worker_config_t;
#pragma once

typedef struct config__t
{
    char *queue_dir;
    size_t workers_count;
    int min_interval_working_with_addr;
} config_t;
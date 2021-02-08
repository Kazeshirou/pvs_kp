#pragma once

struct config_t
{
    char *queue_dir;
    size_t workers_count;
    int min_interval_working_with_addr;
};
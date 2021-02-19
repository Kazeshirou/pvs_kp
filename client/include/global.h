#pragma once

#include <errno.h>
#include <stdio.h>

#include "config.h"
#include "end_marker.h"
#include "peer.h"

worker_config_t g_config; 
peer_t *g_logger;

#define MAX_g_log_message 1024
char g_log_message[MAX_g_log_message];

inline static void send_log()
{
    string_t *slog = string_init2(g_log_message, strlen(g_log_message));
    if (slog)
    {
        add_message(g_logger, slog, LOG_END_MARKER);
        string_clear(slog);
    }
}

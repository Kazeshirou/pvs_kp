#pragma once

#include <errno.h>
#include <stdio.h>

#include "config.h"
#include "end_marker.h"
#include "peer.h"

extern worker_config_t g_config;
extern peer_t*         g_logger;

#define MAX_G_LOG_MESSAGE 2048
extern char g_log_message[MAX_G_LOG_MESSAGE];

inline static void send_log() {
    string_t* slog = string_init2(g_log_message, strlen(g_log_message));
    if (slog) {
        add_message(g_logger, slog, LOG_END_MARKER);
        string_clear(slog);
    }
}

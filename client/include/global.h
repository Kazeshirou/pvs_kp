#pragma once

#include <errno.h>

#include "config.h"
#include "end_marker.h"
#include "peer.h"

worker_config_t g_config; 
peer_t *g_logger;

#define SEND_LOG(log) \
    add_message(g_logger, log, LOG_END_MARKER)

inline static void send_log_char(char *clog)
{
    string_t *log = string_init2(clog, strlen(clog));
    add_message(g_logger, log, LOG_END_MARKER);
    string_clear(log);
}

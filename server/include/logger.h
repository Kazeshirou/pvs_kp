#pragma once

#include <signal.h>
#include <stdio.h>

#include "custom_errors.h"
#include "queue.h"

#define MAX_LOGGER_OUTPUTS 10

typedef struct {
    FILE*                 outputs[MAX_LOGGER_OUTPUTS];
    size_t                current_outputs;
    queue_t               log_queue;
    volatile sig_atomic_t end_flag;
} logger_t;

extern logger_t* logger_;

void log_critical(const char* system, const char* msg);
void log_warning(const char* system, const char* msg);
void log_info(const char* system, const char* msg);

error_code_t init_logger(const char* log_path);
error_code_t add_output(FILE* output);
void         destroy_logger();

int logger_thread(void*);
#include "logger.h"

#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "while_true.h"

typedef char log_t[2000];

static void inner_log(size_t time, const char* level, const char* system,
                      const char* msg) {
    log_t log;
    snprintf(log, sizeof(log), "%ld | %s | %s | %s\n", time, level, system,
             msg);
    queue_push_back(&logger_->log_queue, log, sizeof(log));
}

static size_t get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void log_critical(const char* system, const char* msg) {
    inner_log(get_time(), "CRITICAL", system, msg);
}
void log_warning(const char* system, const char* msg) {
    inner_log(get_time(), "WARNING", system, msg);
}
void log_info(const char* system, const char* msg) {
    inner_log(get_time(), "INFO", system, msg);
}


error_code_t init_logger(const char* log_path) {
    logger_->outputs[logger_->current_outputs] = fopen(log_path, "w");
    if (!logger_->outputs[logger_->current_outputs]) {
        return CE_COMMON;
    }
    logger_->current_outputs++;

    if (queue_init(&logger_->log_queue) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }

    return CE_SUCCESS;
}

error_code_t add_output(FILE* output) {
    if (logger_->current_outputs == MAX_LOGGER_OUTPUTS) {
        return CE_MAX_SIZE;
    }

    logger_->outputs[logger_->current_outputs++] = output;
    return CE_SUCCESS;
}

void destroy_logger() {
    fclose(logger_->outputs[0]);
    queue_destroy(&logger_->log_queue, NULL);
}

int logger_thread(void* ptr) {
    (void)ptr;
    log_t        msg;
    error_code_t cerr;
    WHILE_TRUE() {
        cerr = queue_pop_front(&logger_->log_queue, msg, sizeof(msg));
        if (cerr != CE_SUCCESS) {
            continue;
        }

        for (size_t i = 0; i < logger_->current_outputs; i++) {
            fprintf(logger_->outputs[i], "%s", msg);
        }
    }
    // Костыль, чтоб подождать все сообщения на выходе.
    sleep(1);

    while (queue_try_pop_front(&logger_->log_queue, msg, sizeof(msg)) ==
           CE_SUCCESS) {
        for (size_t i = 0; i < logger_->current_outputs; i++) {
            fprintf(logger_->outputs[i], "%s", msg);
        }
    }

    return 0;
}
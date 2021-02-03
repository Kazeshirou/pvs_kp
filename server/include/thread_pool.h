#pragma once

#include <threads.h>

#include "job.h"
#include "queue.h"

#define WORKERS_COUNT 10

typedef struct {
    thrd_t   td;
    size_t   id;
    queue_t* job_queue;
} worker_t;

typedef struct {
    queue_t  job_queue;
    worker_t workers[WORKERS_COUNT];
} thread_pool_t;

error_code_t thread_pool_init(thread_pool_t* tp);

error_code_t thread_pool_push_job(thread_pool_t* tp, job_t job);

void thread_pool_destroy(thread_pool_t* tp);
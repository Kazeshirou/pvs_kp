#pragma once

#include <signal.h>
#include <threads.h>

#include "job.h"
#include "queue.h"

#define WORKERS_COUNT 10

typedef struct {
    thrd_t                td;
    size_t                id;
    queue_t*              job_queue;
    volatile sig_atomic_t end_flag;
} worker_t;

typedef int (*main_worker_func_t)(void*);

typedef struct {
    main_worker_func_t main_func;
    queue_t            job_queue;
    worker_t           workers[WORKERS_COUNT];
} thread_pool_t;

error_code_t thread_pool_init(thread_pool_t* tp);

error_code_t thread_pool_push_job(thread_pool_t* tp, job_t job);

void thread_pool_destroy(thread_pool_t* tp);
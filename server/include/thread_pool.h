#pragma once

#include <signal.h>
#include <threads.h>

#include "job.h"
#include "queue.h"

struct thread_pool__t;

typedef struct {
    thrd_t                 td;
    size_t                 id;
    queue_t*               job_queue;
    volatile sig_atomic_t  end_flag;
    struct thread_pool__t* tp;
    const void*            worker_info;
} worker_t;

typedef int (*main_worker_func_t)(void*);

typedef struct thread_pool__t {
    main_worker_func_t    main_func;
    queue_t               job_queue;
    destructor_t          job_destructor;
    worker_t*             workers;
    size_t                size;
    volatile sig_atomic_t is_ended;
} thread_pool_t;

error_code_t thread_pool_init(thread_pool_t* tp, size_t size,
                              main_worker_func_t main_func,
                              const void*        worker_info);

error_code_t thread_pool_push_job(thread_pool_t* tp, job_t job);

void thread_pool_destroy(thread_pool_t* tp);

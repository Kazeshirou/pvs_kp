#include "thread_pool.h"

#include <stdlib.h>
#include <unistd.h>

#include "while_true.h"

static int main_worker_func(void* worker_ptr) {
    worker_t* worker = worker_ptr;
    job_t     current_job;
    while (!worker->end_flag) {
        error_code_t error = queue_pop_front(worker->job_queue, &current_job,
                                             sizeof(current_job));
        if (error != CE_SUCCESS) {
            continue;
        }
        job_run(&current_job);
        job_destroy(&current_job);
    }
    worker->tp->is_ended++;
    return 0;
}

error_code_t thread_pool_init(thread_pool_t* tp, size_t size,
                              main_worker_func_t main_func,
                              const void*        worker_info) {
    tp->workers = (worker_t*)malloc(sizeof(worker_t) * size);
    if (!tp->workers) {
        return CE_ALLOC;
    }
    tp->size          = size;
    error_code_t cerr = queue_init(&tp->job_queue);
    if (cerr != CE_SUCCESS) {
        free(tp->workers);
        return cerr;
    }
    int    err;
    size_t i           = 0;
    tp->is_ended       = 0;
    tp->job_destructor = (destructor_t)&job_destroy;
    for (; i < tp->size; i++) {
        tp->workers[i].id          = i;
        tp->workers[i].end_flag    = 0;
        tp->workers[i].job_queue   = &tp->job_queue;
        tp->workers[i].tp          = tp;
        tp->workers[i].worker_info = worker_info;
        if (!main_func) {
            tp->main_func = &main_worker_func;
        } else {
            tp->main_func = main_func;
        }
        err =
            thrd_create(&(tp->workers[i].td), tp->main_func, &(tp->workers[i]));
        if (err != thrd_success) {
            break;
        }
    }
    if (i != tp->size) {
        for (size_t j = 0; j < i; j++) {
            tp->workers[j].end_flag = 1;
        }
        int res;
        for (size_t j = 0; j < i; i++) {
            thrd_join(tp->workers[i].td, &res);
        }
        queue_destroy(&tp->job_queue, tp->job_destructor);
        free(tp->workers);
        return CE_INIT_3RD;
    }
    return CE_SUCCESS;
}

error_code_t thread_pool_push_job(thread_pool_t* tp, job_t job) {
    return queue_push_back(&tp->job_queue, &job, sizeof(job));
}

void thread_pool_destroy(thread_pool_t* tp) {
    for (size_t i = 0; i < tp->size; i++) {
        tp->workers[i].end_flag = 1;
    }
    int res;
    for (size_t i = 0; i < tp->size; i++) {
        thrd_join(tp->workers[i].td, &res);
    }
    queue_destroy(&tp->job_queue, tp->job_destructor);
    free(tp->workers);
}
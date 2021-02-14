#include "thread_pool.h"

#include <stdio.h>
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
        // Обработка возвращённого работой кода ошибки в current_job.err.
        job_destroy(&current_job);
    }
    worker->tp->is_ended++;
    return 0;
}

error_code_t thread_pool_init(thread_pool_t* tp, main_worker_func_t main_func,
                              const void* worker_info) {
    error_code_t cerr = queue_init(&tp->job_queue);
    if (cerr != CE_SUCCESS) {
        // printf("Не удалось проинициализировать очередь работ для пула
        // потоков");
        return cerr;
    }
    int    err;
    size_t i     = 0;
    tp->is_ended = 0;
    for (; i < WORKERS_COUNT; i++) {
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
        err = thrd_create(&(tp->workers[i].td), tp->main_func, tp->workers + i);
        if (err != thrd_success) {
            // printf("Не удалось создать пул потоков");
            break;
        }
        err = thrd_detach(tp->workers[i].td);
        if (err != thrd_success) {
            // printf("Не удалось выполнить detach при создании пула потоков");
            break;
        }
    }
    if (i != WORKERS_COUNT) {
        thread_pool_destroy(tp);
        return CE_INIT_3RD;
    }
    return CE_SUCCESS;
}

error_code_t thread_pool_push_job(thread_pool_t* tp, job_t job) {
    return queue_push_back(&tp->job_queue, &job, sizeof(job));
}

void thread_pool_destroy(thread_pool_t* tp) {
    for (size_t i = 0; i < WORKERS_COUNT; i++) {
        tp->workers[i].end_flag = 1;
    }
    queue_destroy(&tp->job_queue, (destructor_t)&job_destroy);
    // printf("Waiting for workers finish...\n");
    while (tp->is_ended != WORKERS_COUNT) {
        sleep(0.1);
    }
}
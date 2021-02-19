#include "job.h"

#include <stdlib.h>
#include <string.h>

error_code_t job_init(job_t* job, work_function_t work_function, void* arg,
                      size_t arg_size, destructor_t arg_destructor) {
    job->arg = malloc(arg_size);
    if (!job->arg) {
        return CE_ALLOC;
    }

    memcpy(job->arg, arg, arg_size);

    job->work_function  = work_function;
    job->arg_destructor = arg_destructor;
    return CE_SUCCESS;
}

void job_run(job_t* job) {
    job->err = job->work_function(job->arg);
}

void job_destroy(job_t* job) {
    if (job->arg_destructor) {
        job->arg_destructor(job->arg);
    }
    free(job->arg);
}
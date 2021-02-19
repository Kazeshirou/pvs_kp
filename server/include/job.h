#pragma once

#include <stddef.h>

#include "custom_errors.h"
#include "destructor.h"

typedef error_code_t (*work_function_t)(void*);

typedef struct {
    work_function_t work_function;
    void*           arg;
    destructor_t    arg_destructor;
    error_code_t    err;
} job_t;

error_code_t job_init(job_t* job, work_function_t work_function, void* arg,
                      size_t arg_size, destructor_t arg_destructor);

void job_run(job_t* job);

void job_destroy(job_t* job);
#include "logger.h"

#include <stdio.h>

#include "select_fd_storage.h"
#include "errors.h"
#include "while_true.h"
#include "end_marker.h"

int process_logs(queue_t *logs, FILE *f, int worker_fd)
{
    string_t *log = (string_t*) queue_peek(logs);
    while (log)
    {
        printf("[%d] %s\n", worker_fd, log->data);
        fprintf(f, "[%d] %s\n", worker_fd, log->data);

        fflush(f);
        fflush(stdout);

        queue_pop_front(logs);
        log = (string_t*) queue_peek(logs);
    }
    return SUCCESS;
}

int logger_main(const logger_config_t config)
{
    FILE *f;
    f = fopen(config.log_file, "a");
    if (!f)
    {
        return LOG_FILE_OPEN_ERROR;
    }

    int i;
    int workers_count = config.workers_count + 1; // + master

    select_fd_storage_t *storage = storage_init();
    if (!storage)
    {
        return MEMORY_ERROR;
    }

    peer_t **workers = (peer_t**) malloc(sizeof(peer_t*) * workers_count);
    if (!workers)
    {
        storage_clear(storage);
        return MEMORY_ERROR;
    }

    peer_t *worker;
    int worker_idx = 0;

    for (i = 0; i < workers_count; i++)
    {
        worker = peer_init(config.worker_pipe_fds[i], FDT_PIPE);
        if (worker)
        {
            workers[worker_idx++] = worker;
        }
        else
        {
            workers_count--;
        }
    }

    WHILE_TRUE() 
    {        
        select_step(storage, workers, workers_count);

        for (i = 0; i < workers_count; i++)
        {
            fill_messages_out(workers[i], LOG_END_MARKER);
            process_logs(workers[i]->messages_out, f, workers[i]->fd);
        }
    } 

    /*for (i = 0; i < workers_count; i++)
        peer_clear(workers[i]);
    free(workers);
    storage_clear(storage);*/
   
    printf("Процесс логгирования завершен");

    fclose(f);

    return SUCCESS;
}
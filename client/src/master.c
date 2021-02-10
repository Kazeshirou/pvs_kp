#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //pipe
#include <sys/wait.h>
#include <time.h>

#include "master.h"
#include "worker.h"
#include "fileparser.h"
#include "while_true.h"
#include "select_fd_storage.h"
#include "end_marker.h"

/* private */

worker_t pick_worker_by_rr(state_t *state, const char *addr,
                           const worker_t *workers, size_t workers_count)
{
    worker_t worker;
    worker_time_t worker_time;
    tree_t *addr_vs_worker = state->addr_vs_worker;
    int rr_index = state->rr_index;

    worker = workers[rr_index];

    rr_index = (rr_index + 1) % workers_count;
    state->rr_index = rr_index;

    worker_time.worker = worker;
    worker_time.last_time_working = time(NULL);
    TREE_INSERT(addr_vs_worker, addr, worker_time);
    return worker;
}

int is_time_expired(const worker_time_t worker_time, int min_interval_working_with_addr)
{
    return time(NULL) - worker_time.last_time_working > min_interval_working_with_addr;
}

void update_last_time_working(worker_time_t *worker_time)
{
    worker_time->last_time_working = time(NULL);
}

worker_t pick_worker(state_t *state, const worker_t *workers,
                     const char *addr, master_config_t config)
{
    worker_t worker;
    worker_time_t worker_time;
    tree_t *addr_vs_worker = state->addr_vs_worker;
    tree_node_t *node = tree_search(addr_vs_worker, addr);
    if (!node)
    {
        worker = pick_worker_by_rr(state, addr, workers, config.workers_count);
    }
    else
    {
        worker_time = *((worker_time_t*)node->value);
        if (is_time_expired(worker_time, config.min_interval_working_with_addr))
        {
            tree_delete(addr_vs_worker, addr);
            worker = pick_worker_by_rr(state, addr, workers, config.workers_count);
        }
        else
        {
            update_last_time_working(&worker_time);
            worker = worker_time.worker;
        }
    }
    return worker;
}

state_t* state_init()
{
    state_t *state = (state_t*) malloc(sizeof(state_t));
    state->rr_index = 0;
    state->sended_files = TREE_INIT(NULL, NULL, NULL); //set
    state->addr_vs_worker = TREE_INIT(worker_time_t, NULL, NULL);
    return state;
}

void state_clear(state_t *state)
{
    tree_clear(state->addr_vs_worker);
    tree_clear(state->sended_files);
    free(state);
}

worker_t* init_workers(master_config_t *master_config)
{
    int i;
    int pid;
    int exit_code;
    int count = master_config->workers_count;
    int pipe_fds[count][2];
    worker_t worker;
    worker_t *workers = (worker_t*) malloc(sizeof(worker_t) * count);
    worker_config_t worker_config;

    for (i = 0; i < count; i++)
    {
        if (pipe(pipe_fds[i]) < 0)
        {
            printf("pipe()");
            master_config->workers_count--;
            continue;
        }

        pid = fork();
        if (pid < 0)
        {
            master_config->workers_count--;
            printf("fork()");
            continue;
        }
        // child process
        if (pid == 0)
        {
            close(pipe_fds[i][1]); // child can't write

            worker_config.queue_dir = master_config->queue_dir;
            worker_config.parent_pipe_fd = pipe_fds[i][0];

            exit_code = worker_main(worker_config);

            close(pipe_fds[i][0]);
            exit(exit_code);
        }
        // parent process
        else
        {
            close(pipe_fds[i][0]); // parent can't read
            worker.pid = pid;
            worker.peer_write = peer_init(pipe_fds[i][1], FDT_PIPE);
            workers[i] = worker;
        }
    }
    return workers;
}

void clear_workers(worker_t *workers, int size)
{
    int i;
    int status;
    for (i = 0; i < size; i++)
    {
        waitpid(workers[i].pid, &status, 0);
        if (status != 0)
        {
            printf("worker status");
        }
        close(workers[i].peer_write->fd);
        peer_clear(workers[i].peer_write);
    }
    free(workers);
}

int add_filename_to_worker(worker_t *worker, const string_t *filename)
{
    int ret;
    ret = add_message(worker->peer_write, filename, PARENT_MESSAGE_SEP_STR);
    return ret;
}

void add_filenames_to_workers(state_t *state, const queue_t *new_files,
                              worker_t *workers, master_config_t config)

{
    char *addr;
    string_t *filename;
    worker_t worker;
    tree_t *sended_files = state->sended_files;
    queue_node_t *current_file = new_files->front;
    while (current_file)
    {
        filename = ((string_t*)current_file->value);
        if (!tree_search(sended_files, filename->data))
        {
            // send file name
            addr = get_addr(filename);
            worker = pick_worker(state, workers, addr, config);
            add_filename_to_worker(&worker, filename);
            tree_insert(sended_files, filename->data, NULL);
        }
        current_file = current_file->next;
    }
}

peer_t** get_peers_from_workers(worker_t *workers, size_t workers_count)
{
    int i;
    peer_t **peers = (peer_t**) malloc(sizeof(peer_t*) * workers_count);
    for (i = 0; i < workers_count; i++)
    {
        peers[i] = workers[i].peer_write;
    }
    return peers;
}


void dispatch(worker_t *workers, master_config_t config)
{
    int i;
    state_t *state = state_init();
    queue_t *new_files = NULL;
    select_fd_storage_t *storage = storage_init();
    peer_t **peers = get_peers_from_workers(workers, config.workers_count);

    WHILE_TRUE()
    {
        new_files = get_filenames(config.queue_dir);
        if (new_files)
        {
            add_filenames_to_workers(state, new_files, workers, config);
            queue_clear(new_files);
        }

        for (i = 0; i < config.workers_count; i++)
        {
            fill_buffer_in(workers[i].peer_write);
        }

        select_step(storage, peers, config.workers_count);
    }

    state_clear(state);
    storage_clear(storage);
    free(peers);
}

void master_main(master_config_t config)
{
    worker_t *workers = init_workers(&config);
    dispatch(workers, config);
    clear_workers(workers, config.workers_count);

    printf("bye parent!\n");
}


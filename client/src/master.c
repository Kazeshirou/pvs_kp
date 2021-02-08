#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //pipe
#include <sys/wait.h>
#include <time.h>

#include "master.h"
#include "worker.h"
#include "tree.h"
#include "fileparser.h"
#include "while_true.h"
#include "select_fd_storage.h"

/* private */

struct worker_t pick_worker_by_rr(struct state_t *state, char *addr,
                                  struct worker_t *workers, size_t workers_count)
{
    struct worker_t worker;
    struct worker_time_t worker_time;
    struct tree_t *addr_vs_worker = state->addr_vs_worker;
    int rr_index = state->rr_index;

    worker = workers[rr_index];

    rr_index = (rr_index + 1) % workers_count;
    state->rr_index = rr_index;

    worker_time.worker = worker;
    worker_time.last_time_working = time(NULL);
    tree_insert(addr_vs_worker, addr, &worker_time, sizeof(struct worker_t));
    return worker;
}

int is_time_expired(struct worker_time_t worker_time, int min_interval_working_with_addr)
{
    return time(NULL) - worker_time.last_time_working > min_interval_working_with_addr;
}

void update_last_time_working(struct worker_time_t *worker_time)
{
    worker_time->last_time_working = time(NULL);
}

struct worker_t pick_worker(struct state_t *state, struct worker_t *workers,
                            char *addr, struct config_t config)
{
    struct worker_t worker;
    struct worker_time_t worker_time;
    struct tree_t *addr_vs_worker = state->addr_vs_worker;
    struct tree_node_t *node = tree_search(addr_vs_worker, addr);
    if (!node)
    {
        worker = pick_worker_by_rr(state, addr, workers, config.workers_count);
    }
    else
    {
        worker_time = *((struct worker_time_t*)node->value);
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

struct state_t* state_init()
{
    struct state_t *state = (struct state_t*) malloc(sizeof(struct state_t));
    state->rr_index = 0;
    state->sended_files = tree_init();
    state->addr_vs_worker = tree_init();
    return state;
}

void state_clear(struct state_t *state)
{
    tree_clear(state->addr_vs_worker);
    tree_clear(state->sended_files);
    free(state);
}

/* public */

struct worker_t* init_workers(struct config_t *config)
{
    int i;
    int pid;
    int count = config->workers_count;
    int pipe_fds[count][2];
    struct worker_t worker;
    struct worker_t *workers = (struct worker_t*) malloc(sizeof(struct worker_t) * count);
    for (i = 0; i < count; i++)
    {
        if (pipe(pipe_fds[i]) < 0)
        {
            printf("pipe()");
            config->workers_count--;
            continue;
        }

        pid = fork();
        if (pid < 0)
        {
            config->workers_count--;
            printf("fork()");
            continue;
        }
        // child process
        if (pid == 0)
        {
            close(pipe_fds[i][1]); // child can't write
            worker_main(pipe_fds[i][0], *config);
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

void clear_workers(struct worker_t *workers, int size)
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

int add_filename_to_worker(struct worker_t *worker, char *filename)
{
    struct message_t message;
    message.data = filename;
    message.size = strlen(filename);
    return add_message(worker->peer_write, &message);
}

void add_filenames_to_workers(struct state_t *state, struct queue_t *new_files,
                              struct worker_t *workers, struct config_t config)

{
    char *addr;
    char *filename;
    struct worker_t worker;
    struct tree_t *sended_files = state->sended_files;
    struct queue_node_t *current_file = new_files->front;
    while (current_file)
    {
        filename = (char*)current_file->value;
        if (!tree_search(sended_files, filename))
        {
            // send file name
            addr = get_addr(filename);
            worker = pick_worker(state, workers, addr, config);
            add_filename_to_worker(&worker, filename);
            tree_insert(sended_files, filename, NULL, 0);
        }
        current_file = current_file->next;
    }
}

struct peer_t** get_peers_from_workers(struct worker_t *workers, size_t workers_count)
{
    int i;
    struct peer_t **peers = (struct peer_t**) malloc(sizeof(struct peer_t*) * workers_count);
    for (i = 0; i < workers_count; i++)
    {
        peers[i] = workers[i].peer_write;
    }
    return peers;
}


void dispatch(struct worker_t *workers, struct config_t config)
{
    int i;
    struct state_t *state = state_init();
    struct queue_t *new_files = NULL;
    struct select_fd_storage *storage = storage_init();
    struct peer_t **peers = get_peers_from_workers(workers, config.workers_count);

    WHILE_TRUE()
    {
        new_files = get_files_names(config.queue_dir);
        if (new_files)
        {
            add_filenames_to_workers(state, new_files, workers, config);
            queue_clear(new_files, NULL);
        }

        for (i = 0; i < config.workers_count; i++)
        {
            fill_buffer_in(workers[i].peer_write, " ");
        }

        select_step(storage, peers, config.workers_count);
    }

    state_clear(state);
    storage_clear(storage);
    free(peers);
}

void master_main(struct config_t config)
{
    struct worker_t *workers = init_workers(&config);
    dispatch(workers, config);
    clear_workers(workers, config.workers_count);
}


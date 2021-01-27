#include "poll_fd_storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct poll_fd_storage create_poll_fd_storage(const size_t max_size) {
    struct poll_fd_storage storage = {
        .fds      = NULL,
        .msgs     = NULL,
        .size     = 0,
        .max_size = 0,
    };
    storage.fds = (struct pollfd*)calloc(max_size, sizeof(struct pollfd));

    if (!storage.fds) {
        printf("Creating of poll_fd_storage.fds failed");
        return storage;
    }

    storage.msgs = (struct msg*)calloc(max_size, sizeof(struct msg));

    if (!storage.msgs) {
        printf("Creating of poll_fd_storage.msgs failed");
        free(storage.fds);
        return storage;
    }
    for (size_t i = 0; i < max_size; i++) {
        storage.msgs[i] = create_msg(50);
        if (!storage.msgs[i].max_size) {
            printf("Creating of poll_fd_storage.msgs[i] failed");
            free(storage.fds);
            for (size_t j = 0; j < i; j++) {
                free_msg(storage.msgs + j);
            }
            free(storage.msgs);
        }
    }

    storage.max_size = max_size;
    for (size_t i = 0; i < max_size; i++) {
        storage.fds[i].fd = -1;
    }
    return storage;
}

int recreate_poll_fd_storage(struct poll_fd_storage* storage,
                             const size_t            new_max_size) {
    if (new_max_size <= storage->max_size) {
        return 0;
    }

    struct pollfd* new_buf =
        (struct pollfd*)calloc(new_max_size, sizeof(struct pollfd));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return -1;
    }

    struct msg* new_msg_buf =
        (struct msg*)calloc(new_max_size, sizeof(struct msg));
    if (!new_msg_buf) {
        printf("new_msg_buf calloc failed");
        return -1;
    }


    for (size_t i = storage->max_size; i < new_max_size; i++) {
        new_buf[i].fd  = -1;
        new_msg_buf[i] = create_msg(50);
        if (!new_msg_buf[i].max_size) {
            printf("new_msg_buf[i] creating failed");
            return -1;
        }
    }


    if (storage->fds) {
        memcpy(new_buf, storage->fds, storage->size);
        free(storage->fds);
    }

    if (storage->msgs) {
        memcpy(new_msg_buf, storage->msgs, storage->size);
        free(storage->msgs);
    }

    storage->fds      = new_buf;
    storage->msgs     = new_msg_buf;
    storage->max_size = new_max_size;
    return 1;
}

int add_poll_fd_to_storage(struct poll_fd_storage* storage,
                           const struct pollfd     fd) {
    size_t new_size = storage->size + 1;
    if (new_size > storage->max_size) {
        if (recreate_poll_fd_storage(storage, new_size + 50) < 0) {
            return -1;
        }
    }

    storage->fds[storage->size] = fd;
    storage->size               = new_size;
    return storage->size;
}

void free_poll_fd_storage(struct poll_fd_storage* storage) {
    free(storage->fds);
    for (size_t i = 0; i < storage->max_size; i++) {
        free_msg(storage->msgs + i);
    }
    free(storage->msgs);
}

void compress_poll_fd_storage(struct poll_fd_storage* storage) {
    for (size_t i = 0; i < storage->size; i++) {
        if (storage->fds[i].fd == -1) {
            free_msg(storage->msgs + i);
            for (size_t j = i; j < storage->size; j++) {
                storage->fds[j].fd = storage->fds[j + 1].fd;
                storage->msgs[j]   = storage->msgs[j + 1];
            }
            i--;
            storage->size--;
        }
    }
}
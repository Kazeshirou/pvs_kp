#include "smtp_server.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "msg.h"
#include "server_info.h"
#include "thread_pool.h"
#include "while_true.h"

void set_socket_unblock(const int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

error_code_t create_server_socket(const int    port,
                                  const size_t backlog_queue_size,
                                  int*         server_fd) {
    *server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        perror("listen_fd socket failed");
        return CE_INIT_3RD;
    }

    // Для повторного использования локального адреса при перезапуске
    // сервера до истечения требуемого времени ожидания.
    int on = 1;
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on,
                   sizeof(on)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(*server_fd);
        return CE_COMMON;
    }

    // Сделаем сокет не блокируемым.
    set_socket_unblock(*server_fd);

    struct sockaddr_in6 listener_addr;
    memset(&listener_addr, 0, sizeof(listener_addr));
    listener_addr.sin6_family = AF_INET6;
    listener_addr.sin6_port   = htons(port);
    if (bind(*server_fd, (struct sockaddr*)&listener_addr,
             sizeof(listener_addr)) < 0) {
        perror("bind() failed");
        close(*server_fd);
        return CE_COMMON;
    }

    if (listen(*server_fd, backlog_queue_size) < 0) {
        perror("listen() failed");
        close(*server_fd);
        return CE_COMMON;
    }

    return CE_SUCCESS;
}

error_code_t process_listener(const int listener_fd, int* new_client_fd) {
    printf("  Listening socket is readable\n");

    *new_client_fd = accept(listener_fd, NULL, NULL);
    if (*new_client_fd < 0) {
        if (errno != EWOULDBLOCK) {
            perror("  accept() failed");
        }
        return CE_COMMON;
    }

    // Сделаем сокет не блокируемым.
    set_socket_unblock(*new_client_fd);

    printf("  New incoming connection - %d\n", *new_client_fd);
    return CE_SUCCESS;
}

error_code_t process_poll(server_info_t* server_info) {
    // Есть сокеты, готовые к чтению.
    msg_t msg;
    if (msg_init(&msg, 50) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }
    int is_closed;
    int need_compress = 0;
    for (int i = 0; i < server_info->size; i++) {
        if (server_info->fds[i].revents == 0)
            continue;

        if (server_info->fds[i].revents & POLLIN) {
            printf("  Descriptor %d is readable\n", server_info->fds[i].fd);

            if (msg_recv_one(&msg, server_info->fds[i].fd, &is_closed) !=
                CE_SUCCESS) {
                continue;
            }

            if (is_closed) {
                server_info->clients[i]->need_send = 1;
                close(server_info->fds[i].fd = -1);
                need_compress = 1;
                continue;
            }

            client_process_recv(server_info->clients[i], &msg);
            if (server_info->clients[i]->closed) {
                close(server_info->fds[i].fd = -1);
                need_compress = 1;
            }
            continue;
        }

        if ((server_info->fds[i].revents & POLLOUT) &&
            server_info->clients[i]->need_send) {
            printf("  Descriptor %d is writeable\n", server_info->fds[i].fd);

            if (msg_send_one(&(server_info->clients[i]->msg_for_sending),
                             server_info->fds[i].fd) != CE_SUCCESS) {
                continue;
            }
            if (server_info->clients[i]->msg_for_sending.size) {
                continue;
            }
            client_process_send(server_info->clients[i]);
            if (server_info->clients[i]->closed) {
                close(server_info->fds[i].fd = -1);
                need_compress = 1;
            }
            continue;
        }

        client_process_check_timeout(server_info->clients[i]);
        if (server_info->clients[i]->closed) {
            close(server_info->fds[i].fd = -1);
            need_compress = 1;
        }
    }

    if (need_compress) {
        server_info_compress(server_info);
    }
    return CE_SUCCESS;
}

static int main_worker_func(void* worker_ptr) {
    worker_t*     worker = worker_ptr;
    server_info_t server_info;
    if (server_info_init(&server_info, 50) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }

    int          new_client_fd;
    error_code_t cerr;
    int          poll_res;
    while (!worker->end_flag) {
        cerr = queue_try_pop_front(worker->job_queue, &new_client_fd,
                                   sizeof(new_client_fd));
        if (cerr == CE_SUCCESS) {
            server_info_add_client(&server_info, new_client_fd);
        }
        poll_res = poll(server_info.fds, server_info.size, 1 * 1000);
        if (poll_res < 0) {
            perror("  poll() failed");
            continue;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            printf("  poll() timed out\n");
            continue;
        }
        process_poll(&server_info);
    }
    server_info_destroy(&server_info);
    worker->tp->is_ended++;
    printf("thread %ld %ld finished\n", worker->id, worker->td);
    return CE_SUCCESS;
}

void smtp_server(const smtp_server_cfg_t cfg) {
    int          listener_fd;
    error_code_t cerr =
        create_server_socket(cfg.port, cfg.backlog_queue_size, &listener_fd);
    if (cerr != CE_SUCCESS) {
        return;
    }

    thread_pool_t tp;
    cerr = thread_pool_init(&tp, main_worker_func);
    if (cerr != CE_SUCCESS) {
        return;
    }

    // Настройка poll.
    struct pollfd listener_poll_fd;
    listener_poll_fd.fd     = listener_fd;
    listener_poll_fd.events = POLLIN;

    int new_client_fd;
    WHILE_TRUE() {
        printf("Waiting on poll()...\n");
        int poll_res = poll(&listener_poll_fd, 1, -1);

        if (poll_res < 0) {
            perror("  poll() failed");
            continue;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            printf("  poll() timed out.  End program.\n");
            continue;
        }

        if (listener_poll_fd.revents == 0)
            continue;

        if (listener_poll_fd.revents != POLLIN) {
            continue;
        }

        if (process_listener(listener_fd, &new_client_fd) != CE_SUCCESS) {
            continue;
        }

        queue_push_back(&tp.job_queue, &new_client_fd, sizeof(new_client_fd));
    }
    printf("smtp server stop\n");
    thread_pool_destroy(&tp);
}
#include "smtp_server.h"

#include <errno.h>
#define __USE_MISC
#include <grp.h>
#undef __USE_MISC
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger.h"
#include "msg.h"
#include "server_info.h"
#include "smtp_definitions.h"
#include "thread_pool.h"
#include "while_true.h"

static char for_log_msg[1000];

void set_socket_unblock(const int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

static error_code_t droproot(const char* username) {
    struct passwd* pw = NULL;

    pw = getpwnam(username);
    if (pw) {
        if (initgroups(pw->pw_name, pw->pw_gid) != 0 ||
            setgid(pw->pw_gid) != 0 || setuid(pw->pw_uid) != 0) {
            snprintf(for_log_msg, sizeof(for_log_msg),
                     "Невозможно поменять пользователя на '%.32s' uid=%lu "
                     "gid=%lu: %s",
                     username, (unsigned long)pw->pw_uid,
                     (unsigned long)pw->pw_gid, strerror(errno));
            log_critical("smtp_server", for_log_msg);
            return CE_COMMON;
        }
    } else {
        snprintf(for_log_msg, sizeof(for_log_msg),
                 "Не удалось найти пользователя '%.32s'", username);
        log_critical("smtp_server", for_log_msg);
        return CE_COMMON;
    }
    return CE_SUCCESS;
}

static error_code_t create_dirs(const char* local_maildir,
                                const char* client_maildir) {
    struct stat st = {0};
    if (stat(local_maildir, &st) < 0) {
        if (mkdir(local_maildir, 0777) < 0) {
            snprintf(for_log_msg, sizeof(for_log_msg),
                     "Не удалось создать папку '%s'", local_maildir);
            log_critical("smtp_server", for_log_msg);
            return CE_COMMON;
        }
    }
    char dir[256];
    snprintf(dir, sizeof(dir), "%s%s", local_maildir, "tmp");
    if (stat(dir, &st) < 0) {
        if (mkdir(dir, 0777) < 0) {
            snprintf(for_log_msg, sizeof(for_log_msg),
                     "Не удалось создать папку '%s'", dir);
            log_critical("smtp_server", for_log_msg);
            return CE_COMMON;
        }
    }
    if (stat(client_maildir, &st) < 0) {
        if (mkdir(client_maildir, 0777) < 0) {
            snprintf(for_log_msg, sizeof(for_log_msg),
                     "Не удалось создать папку '%s'", client_maildir);
            log_critical("smtp_server", for_log_msg);
            return CE_COMMON;
        }
    }

    return CE_SUCCESS;
}

error_code_t create_server_socket(const int port, const char* address,
                                  const size_t backlog_queue_size,
                                  int*         server_fd) {
    *server_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        snprintf(for_log_msg, sizeof(for_log_msg),
                 "Не удалось создать слушающий сокет: %s", strerror(errno));
        log_critical("smtp_server", for_log_msg);
        return CE_INIT_3RD;
    }

    // Для повторного использования локального адреса при перезапуске
    // сервера до истечения требуемого времени ожидания.
    int on = 1;
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on,
                   sizeof(on)) < 0) {
        snprintf(for_log_msg, sizeof(for_log_msg),
                 "setsockopt(SO_REUSEADDR) провален: %s", strerror(errno));
        log_critical("smtp_server", for_log_msg);
        close(*server_fd);
        return CE_COMMON;
    }

    // Сделаем сокет не блокируемым.
    set_socket_unblock(*server_fd);

    struct sockaddr_in6 listener_addr;
    memset(&listener_addr, 0, sizeof(listener_addr));
    listener_addr.sin6_family = AF_INET6;
    listener_addr.sin6_port   = htons(port);
    if (inet_pton(AF_INET6, address, &listener_addr.sin6_addr) != 1) {
        snprintf(for_log_msg, sizeof(for_log_msg),
                 "Введён некорректный адрес %s", address);
        log_critical("smtp_server", for_log_msg);
        close(*server_fd);
        return CE_COMMON;
    }

    if (bind(*server_fd, (struct sockaddr*)&listener_addr,
             sizeof(listener_addr)) < 0) {
        snprintf(for_log_msg, sizeof(for_log_msg), "bind() провален: %s",
                 strerror(errno));
        log_critical("smtp_server", for_log_msg);
        close(*server_fd);
        return CE_COMMON;
    }

    if (listen(*server_fd, backlog_queue_size) < 0) {
        snprintf(for_log_msg, sizeof(for_log_msg), "listen() провален: %s",
                 strerror(errno));
        log_critical("smtp_server", for_log_msg);
        close(*server_fd);
        return CE_COMMON;
    }

    return CE_SUCCESS;
}

error_code_t process_listener(const int listener_fd, int* new_client_fd) {
    *new_client_fd = accept(listener_fd, NULL, NULL);
    if (*new_client_fd < 0) {
        if (errno != EWOULDBLOCK) {
            snprintf(for_log_msg, sizeof(for_log_msg), "accept() провален: %s",
                     strerror(errno));
            log_critical("smtp_server", for_log_msg);
        }
        return CE_COMMON;
    }

    // Сделаем сокет не блокируемым.
    set_socket_unblock(*new_client_fd);

    snprintf(for_log_msg, sizeof(for_log_msg), "Новое соединение: %d",
             *new_client_fd);
    log_info("smtp_server", for_log_msg);
    return CE_SUCCESS;
}

error_code_t process_poll(server_info_t* server_info) {
    char for_log_msg[1000];
    // Есть сокеты, готовые к чтению.
    msg_t msg;
    if (msg_init(&msg, SMTP_TEXT_LINE_MAX_SIZE) != CE_SUCCESS) {
        return CE_INIT_3RD;
    }
    int is_closed;
    int need_compress = 0;
    for (int i = 0; i < server_info->size; i++) {
        if (server_info->fds[i].revents == 0)
            continue;

        if (server_info->fds[i].revents & POLLIN) {
            if (msg_recv_one(&msg, server_info->fds[i].fd, &is_closed) !=
                CE_SUCCESS) {
                continue;
            }

            if (is_closed) {
                close(server_info->fds[i].fd);
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "Соединение с клиентом %d разорвано у потока %ld ",
                         server_info->fds[i].fd, server_info->id);
                log_info("smtp_server", for_log_msg);
                server_info->fds[i].fd = -1;
                need_compress          = 1;
                continue;
            }

            client_process_recv(server_info->clients[i], &msg);
            if (server_info->clients[i]->closed) {
                close(server_info->fds[i].fd);
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "Соединение с клиентом %d у потока %ld закрыто",
                         server_info->fds[i].fd, server_info->id);
                log_info("smtp_server", for_log_msg);
                server_info->fds[i].fd = -1;
                need_compress          = 1;
            } else {
                if (server_info->clients[i]->need_send) {
                    server_info->fds[i].events = POLLIN | POLLOUT;
                } else {
                    server_info->fds[i].events = POLLIN;
                }
            }
            continue;
        }

        if ((server_info->fds[i].revents & POLLOUT) &&
            server_info->clients[i]->need_send) {
            if (msg_send_one(&(server_info->clients[i]->msg_for_sending),
                             server_info->fds[i].fd) != CE_SUCCESS) {
                continue;
            }
            if (server_info->clients[i]->msg_for_sending.size) {
                continue;
            }
            client_process_send(server_info->clients[i]);
            if (server_info->clients[i]->closed) {
                close(server_info->fds[i].fd);
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "Соединение с клиентом %d у потока %ld закрыто",
                         server_info->fds[i].fd, server_info->id);
                log_info("smtp_server", for_log_msg);
                server_info->fds[i].fd = -1;
                need_compress          = 1;
            } else {
                if (server_info->clients[i]->need_send) {
                    server_info->fds[i].events = POLLIN | POLLOUT;
                } else {
                    server_info->fds[i].events = POLLIN;
                }
            }
            continue;
        }

        client_process_check_timeout(server_info->clients[i],
                                     server_info->timeout);
        if (server_info->clients[i]->closed) {
            close(server_info->fds[i].fd);
            snprintf(for_log_msg, sizeof(for_log_msg),
                     "Соединение с клиентом %d у потока %ld закрыто",
                     server_info->fds[i].fd, server_info->id);
            log_info("smtp_server", for_log_msg);
            server_info->fds[i].fd = -1;
            need_compress          = 1;
        }
    }

    msg_destroy(&msg);

    if (need_compress) {
        server_info_compress(server_info);
    }
    return CE_SUCCESS;
}

static int main_worker_func(void* worker_ptr) {
    char                     for_log_msg[1000];
    worker_t*                worker = worker_ptr;
    const smtp_server_cfg_t* cfg    = worker->worker_info;
    snprintf(for_log_msg, sizeof(for_log_msg), "Поток %ld %ld создан",
             worker->id, worker->td);
    log_info("smtp_server", for_log_msg);
    server_info_t server_info;
    if (server_info_init(&server_info, cfg, 50, worker->td, worker->id) !=
        CE_SUCCESS) {
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
            snprintf(for_log_msg, sizeof(for_log_msg), "Клиент %d у потока %ld",
                     new_client_fd, worker->id);
            log_info("smtp_server", for_log_msg);
        }
        if (!server_info.size) {
            sleep(0.1);
            continue;
        }
        poll_res = poll(server_info.fds, server_info.size, 100);
        if (poll_res < 0) {
            if (errno != EINTR) {
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "Ошибка poll() клиентских сокетов в потоке %lu: %s",
                         worker->id, strerror(errno));
                log_warning("smtp_server", for_log_msg);
            } else {
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "poll() прерван сигналом в потоке %lu", worker->id);
                log_info("smtp_server", for_log_msg);
            }
            continue;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            continue;
        }
        process_poll(&server_info);
    }
    snprintf(for_log_msg, sizeof(for_log_msg), "Поток %ld %ld завершён",
             worker->id, worker->td);
    log_info("smtp_server", for_log_msg);
    server_info_destroy(&server_info);
    worker->tp->is_ended++;
    return CE_SUCCESS;
}

void smtp_server(const smtp_server_cfg_t cfg) {
    int          listener_fd;
    error_code_t cerr = create_server_socket(
        cfg.port, cfg.address, cfg.backlog_queue_size, &listener_fd);
    if (cerr != CE_SUCCESS) {
        return;
    }

    if (strlen(cfg.user)) {
        cerr = droproot(cfg.user);
        if (cerr != CE_SUCCESS) {
            return;
        }
    }

    cerr = create_dirs(cfg.local_maildir, cfg.client_maildir);
    if (cerr != CE_SUCCESS) {
        return;
    }

    thread_pool_t tp;
    cerr = thread_pool_init(&tp, cfg.thread_pool_size, main_worker_func, &cfg);
    if (cerr != CE_SUCCESS) {
        log_critical("smtp_server",
                     "Не удалось проинициализировать пул потоков");
        return;
    }
    tp.job_destructor = NULL;

    // Настройка poll.
    struct pollfd listener_poll_fd;
    listener_poll_fd.fd     = listener_fd;
    listener_poll_fd.events = POLLIN;

    int new_client_fd;
    WHILE_TRUE() {
        int poll_res = poll(&listener_poll_fd, 1, 1000);

        if (poll_res < 0) {
            if (errno != EINTR) {
                snprintf(for_log_msg, sizeof(for_log_msg),
                         "Ошибка poll() для главного сокета: %s",
                         strerror(errno));
                log_warning("smtp_server", for_log_msg);
            } else {
                log_info("smtp_server", "poll() прерван сигналом");
            }
            continue;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            continue;
        }

        if (listener_poll_fd.revents != POLLIN) {
            continue;
        }

        if (process_listener(listener_fd, &new_client_fd) != CE_SUCCESS) {
            continue;
        }

        queue_push_back(&tp.job_queue, &new_client_fd, sizeof(new_client_fd));
    }
    // printf("smtp server stop\n");
    thread_pool_destroy(&tp);
}

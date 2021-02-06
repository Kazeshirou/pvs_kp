#include "smtp_server.h"

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
// #include <sys/types.h>
#include <unistd.h>

#include "msg.h"
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

error_code_t process_client(server_info_t* storage, const size_t i) {
    struct msg msg = recv_one_message(storage->fds[i].fd);
    if (!msg.size) {
        free_msg(&msg);
        return CE_COMMON;
    }

    printf("  %ld bytes received:\n", msg.size);
    printf("   -> %s\n", msg.text);

    if (!msg.size ||
        (add_text_to_message(storage->msgs + i, msg.text, msg.size) < 0)) {
        perror("  add_text_to_msg() failed");
        free_msg(&msg);
        return CE_COMMON;
    }

    char* full_msg = get_msg(storage->msgs + i);
    if (!full_msg) {
        return CE_COMMON;
    }

    // Собрано полное сообщение (получен маркер конца сообщения).
    // Отправка данных обратно на сервер.
    printf("  Finished msg has found:\n");
    printf("   -> %s\n", full_msg);
    int send_res = send(storage->fds[i].fd, full_msg, strlen(full_msg), 0);
    free_msg(&msg);
    if (send_res < 0) {
        perror("  send() failed");
        return CE_COMMON;
    }
    return CE_SUCCESS;
}

error_code_t process_poll_fds(server_info_t* storage) {
    // Есть сокеты, готовые к чтению.
    int current_size = storage->size;
    int compress     = 0;
    for (int i = 0; i < current_size; i++) {
        if (storage->fds[i].revents == 0)
            continue;

        if (storage->fds[i].revents != POLLIN) {
            printf("  Error on descriptor %d! revents = %d\n",
                   storage->fds[i].fd, storage->fds[i].revents);
            close(storage->fds[i].fd);
            storage->fds[i].fd = -1;
            compress           = 1;
            continue;
        }

        printf("  Descriptor %d is readable\n", storage->fds[i].fd);
        if (process_client(storage, i) < 0) {
            close(storage->fds[i].fd);
            storage->fds[i].fd = -1;
            compress           = 1;
        }
    }

    if (compress) {
        compress = 0;
        compress_poll_fd_storage(storage);
    }

    return 1;
}

void smtp_server(const smtp_server_cfg_t cfg) {
    int          listener_fd;
    error_code_t cerr =
        create_server_socket(cfg.port, cfg.backlog_queue_size, &listener_fd);
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

        // push client to client queue
    }
}

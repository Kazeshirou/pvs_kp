#include "create_server_socket.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int create_server_socket(const int port, const size_t connection_queue_size) {
    int listener_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listener_fd < 0) {
        perror("listen_fd socket failed");
        return -1;
    }

    // Для повторного использования локального адреса при перезапуске
    // сервера до истечения требуемого времени ожидания.
    int on = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on,
                   sizeof(on)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(listener_fd);
        return -1;
    }

    // Сделаем сокет не блокируемым.
    if (ioctl(listener_fd, FIONBIO, (char*)&on) < 0) {
        perror("ioctl() failed");
        close(listener_fd);
        return -1;
    }

    struct sockaddr_in6 listener_addr;
    memset(&listener_addr, 0, sizeof(listener_addr));
    listener_addr.sin6_family = AF_INET6;
    listener_addr.sin6_port   = htons(port);
    if (bind(listener_fd, (struct sockaddr*)&listener_addr,
             sizeof(listener_addr)) < 0) {
        perror("bind() failed");
        close(listener_fd);
        return -1;
    }

    if (listen(listener_fd, connection_queue_size) < 0) {
        perror("listen() failed");
        close(listener_fd);
        return -1;
    }

    return listener_fd;
}
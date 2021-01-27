#include <stdlib.h>  // atoi
#include <unistd.h>  // close

#include "checkoptn.h"

#include "create_server_socket.h"
#include "echo_server.h"
#include "end_marker.h"
#include "end_program_handler.h"
#include "while_true.h"

#define DEFAULT_PORT          49001
#define CONNECTION_QUEUE_SIZE 100

int         while_true = 1;
const char* end_marker = "123";

int main(int argc, char* argv[]) {
    int optct = optionProcess(&serverOptions, argc, argv);
    argc -= optct;
    argv += optct;

    if (set_end_program_handler() < 0) {
        return 0;
    }

    int port = DEFAULT_PORT;
    if (COUNT_OPT(PORT)) {
        port = OPT_VALUE_PORT;
    }
    printf("port = %d\n", port);

    int listener_fd = create_server_socket(port, CONNECTION_QUEUE_SIZE);
    if (listener_fd < 0) {
        return 0;
    }

    echo_server(listener_fd);

    close(listener_fd);

    return 0;
}
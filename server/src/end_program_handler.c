#include "end_program_handler.h"

#include <signal.h>
#include <stdio.h>

#include "while_true.h"

void end_program_handler(int signum) {
    (void)signum;
    printf(" Ending program by signal...\n");
    while_true = 0;

    return;
}

int set_end_program_handler() {
    if (signal(SIGINT, end_program_handler) == SIG_ERR) {
        perror("Can't register end program handler");
        return -1;
    }
    if (signal(SIGQUIT, end_program_handler) == SIG_ERR) {
        perror("Can't register end program handler");
        return -1;
    }

    return 1;
}
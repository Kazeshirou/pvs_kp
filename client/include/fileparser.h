#pragma once

#include "message.h"
#include "queue.h"

#define SERVER_ADDR_SEP '.'
#define FILENAME_SEP '/'
#define FILENAME_SEP_STR "/"

struct queue_t* get_files_names(char *path_to_dir);

char* get_addr(char *filename);
void free_addr(char *addr);

struct message_t parse_message(char *filename);

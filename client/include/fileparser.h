#pragma once

#include "mstring.h"
#include "queue.h"
#include "SMTP_message.h"

#define SERVER_ADDR_SEP '.'
#define FILENAME_SEP '/'
#define FILENAME_SEP_STR "/"

queue_t* get_filenames(const char *path_to_dir);

char* get_addr(const string_t *filename);
void free_addr(char *addr);

SMTP_message_t* parse_message(const char *queue_dir, const string_t *filename);

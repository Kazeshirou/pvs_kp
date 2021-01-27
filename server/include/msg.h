#pragma once

#include <stddef.h>

struct msg {
    char*  text;
    size_t size;
    size_t max_size;
};

struct msg create_msg(const size_t max_size);
int        recreate_msg(struct msg* msg, const size_t new_max_size);
int        add_text_to_message(struct msg* msg, const char* buf, size_t size);
void       free_msg(struct msg* msg);
struct msg recv_one_message(int fd);
char*      get_msg(struct msg* const msg);
#pragma once

#include "custom_errors.h"
#include "msg.h"

typedef enum {
    DOMAIN_TYPE_IPV4 = 0,
    DOMAIN_TYPE_IPV6 = 1,
    DOMAIN_TYPE_HOST = 2
} DOMAIN_TYPE;

typedef struct {
    msg_t       local_part;
    msg_t       domain;
    DOMAIN_TYPE domain_type;
} receiver_t;


typedef struct {
    const char* domain;
    const char* local_maildir;
    const char* client_maildir;
    size_t      pid;
    size_t      tid;
    int*        N;
    const char* hostname;
} mail_writer_t;

error_code_t write_mail(mail_writer_t* mw, receiver_t* to, size_t to_count,
                        msg_t* text);
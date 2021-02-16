#pragma once

#include "custom_errors.h"
#include "msg.h"

typedef struct {
    msg_t local_part;
    msg_t domen;
} receiver_t;


typedef struct {
    const char* domain;
    const char* local_maildir;
    const char* client_maildir;
    int         pid;
    int         tid;
    int*        N;
    const char* hostname;
} mail_writer_t;

error_code_t write_mail(mail_writer_t* mw, receiver_t* to, size_t to_count,
                        msg_t* text);
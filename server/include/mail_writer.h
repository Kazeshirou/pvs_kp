#pragma once

#include "custom_errors.h"
#include "msg.h"

typedef struct {
    const char* domain;
    const char* local_maildir;
    const char* client_maildir;
} mail_writer_t;

error_code_t write_mail(mail_writer_t* mw, const char* filename, msg_t* text);
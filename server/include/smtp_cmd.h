#pragma once

#include "custom_errors.h"
#include "pcre.h"

typedef enum {
    SMTP_CMD_UNKNOW   = -1,
    SMTP_CMD_HELO     = 0,
    SMTP_CMD_EHLO     = 1,
    SMTP_CMD_MAIL     = 2,
    SMTP_CMD_RCPT     = 3,
    SMTP_CMD_VRFY     = 4,
    SMTP_CMD_RSET     = 5,
    SMTP_CMD_QUIT     = 6,
    SMTP_CMD_DATA     = 7,
    SMTP_CMD_END_DATA = 8
} SMTP_CMD;

typedef struct {
    SMTP_CMD    type;
    const char* re;
    pcre*       re_compiled;
    pcre_extra* extra;
} smtp_cmd_t;

error_code_t smtp_cmd_init();
error_code_t smtp_cmd_check(SMTP_CMD cmd, char* tested_line, int* sub_str,
                            int sub_str_size, int* pcre_exec_ret);
error_code_t smtp_cmd_get_substring(char* tested_line, int* sub_str,
                                    int pcre_exec_ret, int index, char* buf,
                                    size_t buf_size);
error_code_t smtp_cmd_get_named_substring(SMTP_CMD cmd, char* tested_line,
                                          int* sub_str, int pcre_exec_ret,
                                          char* name, char* buf,
                                          size_t buf_size);
void         smtp_cmd_destroy();
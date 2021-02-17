#pragma once

#include "custom_errors.h"
#include "pcre.h"

#define MI_MAIL_FROM_REVERSE_PATH_INDEX           1
#define MI_RCPT_TO_FULL_INDEX                     1
#define MI_RCPT_TO_POSTMASTER_FULL_INDEX          2
#define MI_RCPT_TO_POSTMASTER_FULL_DOMAIN_INDEX   3
#define MI_RCPT_TO_POSTMASTER_INDEX               9
#define MI_RCPT_TO_FORWARD_PATH_FULL              28
#define MI_RCPT_TO_FORWARD_PATH_LOCAL_PART_INDEX  29
#define MI_RCPT_TO_FORWARD_PATH_DOMAIN_INDEX      38
#define MI_RCPT_TO_FORWARD_PATH_DOMAIN_HOST_INDEX 39
#define MI_RCPT_TO_FORWARD_PATH_DOMAIN_IPV4_INDEX 47
#define MI_RCPT_TO_FORWARD_PATH_DOMAIN_IPV6_INDEX 57
#define MI_RCPT_TO_FORWARD_PATH_INDEX             1
#define MI_EHLO_INFO_INDEX                        1
#define MI_HELO_INFO_INDEX                        1

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

#define SUB_STR_COUNT 250
typedef struct {
    SMTP_CMD cmd;
    char*    tested_line;
    int      sub_str[SUB_STR_COUNT];
    int      sub_str_count;
} match_info_t;

error_code_t smtp_cmd_init();
error_code_t smtp_cmd_check(SMTP_CMD cmd, match_info_t* match_info);
error_code_t smtp_cmd_get_substring(match_info_t* match_info, int index,
                                    char* buf, size_t buf_size);
error_code_t smtp_cmd_get_named_substring(match_info_t* match_info, char* name,
                                          char* buf, size_t buf_size);
void         smtp_cmd_destroy();
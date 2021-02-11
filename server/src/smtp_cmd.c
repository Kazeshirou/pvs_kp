#include "smtp_cmd.h"

#include <stdio.h>
#include <string.h>

#include "re_definitions.h"

#define SMTP_CMD_COUNT    9
#define SMTP_CMD_RE_FLAGS PCRE_CASELESS | PCRE_MULTILINE

static smtp_cmd_t smtp_cmds_[SMTP_CMD_COUNT] = {
    {.type = SMTP_CMD_HELO, .re = RE_HELO, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_EHLO, .re = RE_EHLO, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_MAIL, .re = RE_MAIL, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_RCPT, .re = RE_RCPT, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_VRFY, .re = RE_VRFY, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_RSET, .re = RE_RSET, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_QUIT, .re = RE_QUIT, .re_compiled = NULL, .extra = NULL},
    {.type = SMTP_CMD_DATA, .re = RE_DATA, .re_compiled = NULL, .extra = NULL},
    {.type        = SMTP_CMD_END_DATA,
     .re          = RE_END_DATA,
     .re_compiled = NULL,
     .extra       = NULL}};

error_code_t smtp_cmd_init() {
    int         pcre_error_offset;
    const char* pcre_error_str;
    for (size_t i = 0; i < SMTP_CMD_COUNT; i++) {
        smtp_cmds_[i].re_compiled =
            pcre_compile(smtp_cmds_[i].re, SMTP_CMD_RE_FLAGS, &pcre_error_str,
                         &pcre_error_offset, NULL);
        if (!smtp_cmds_[i].re_compiled) {
            printf("ERROR: Could not compile '%s': %s\n", smtp_cmds_[i].re,
                   pcre_error_str);
            for (size_t j = 0; j < i; j++) {
                pcre_free(smtp_cmds_[j].re_compiled);
            }
            return CE_INIT_3RD;
        }
        smtp_cmds_[i].extra =
            pcre_study(smtp_cmds_[i].re_compiled, 0, &pcre_error_str);
        if (pcre_error_str) {
            printf("ERROR: Could not study '%s': %s\n", smtp_cmds_[i].re,
                   pcre_error_str);
            for (size_t j = 0; j < i; j++) {
                pcre_free(smtp_cmds_[j].re_compiled);
                if (smtp_cmds_[j].extra != NULL) {
#ifdef PCRE_CONFIG_JIT
                    pcre_free_study(smtp_cmds_[j].extra);
#else
                    pcre_free(smtp_cmds_[j].extra);
#endif
                }
            }
            return CE_INIT_3RD;
        }
    }
    return CE_SUCCESS;
}

error_code_t smtp_cmd_check(SMTP_CMD cmd, char* tested_line, int* sub_str,
                            int sub_str_size, int* pcre_exec_ret) {
    if ((cmd < 0) || (cmd > SMTP_CMD_END_DATA)) {
        return CE_INVALID_ARG;
    }
    *pcre_exec_ret = pcre_exec(
        smtp_cmds_[cmd].re_compiled, smtp_cmds_[cmd].extra, tested_line,
        strlen(tested_line), 0, 0, sub_str, sub_str_size);
    if (*pcre_exec_ret < 0) {
        switch (*pcre_exec_ret) {
            case PCRE_ERROR_NOMATCH:
                printf("String did not match the pattern\n");
                break;
            case PCRE_ERROR_NULL:
                printf("Something was null\n");
                break;
            case PCRE_ERROR_BADOPTION:
                printf("A bad option was passed\n");
                break;
            case PCRE_ERROR_BADMAGIC:
                printf("Magic number bad (compiled re corrupt?)\n");
                break;
            case PCRE_ERROR_UNKNOWN_NODE:
                printf("Something kooky in the compiled re\n");
                break;
            case PCRE_ERROR_NOMEMORY:
                printf("Ran out of memory\n");
                break;
            default:
                printf("Unknown error\n");
                break;
        }
        return CE_COMMON;
    }

    if (*pcre_exec_ret == 0) {
        printf("But too many substrings were found to fit in "
               "subStrVec!\n");
        // Set rc to the max number of substring matches possible.
        *pcre_exec_ret = sub_str_size / 3;
    }

    return CE_SUCCESS;
}

error_code_t smtp_cmd_get_substring(char* tested_line, int* sub_str,
                                    int pcre_exec_ret, int index, char* buf,
                                    size_t buf_size) {
    pcre_copy_substring(tested_line, sub_str, pcre_exec_ret, index, buf,
                        buf_size);
    return CE_SUCCESS;
}

error_code_t smtp_cmd_get_named_substring(SMTP_CMD cmd, char* tested_line,
                                          int* sub_str, int pcre_exec_ret,
                                          char* name, char* buf,
                                          size_t buf_size) {
    pcre_copy_named_substring(smtp_cmds_[cmd].re_compiled, tested_line, sub_str,
                              pcre_exec_ret, name, buf, buf_size);
    return CE_SUCCESS;
}

void smtp_cmd_destroy() {
    for (size_t i = 0; i < SMTP_CMD_COUNT; i++) {
        pcre_free(smtp_cmds_[i].re_compiled);
        if (smtp_cmds_[i].extra != NULL) {
#ifdef PCRE_CONFIG_JIT
            pcre_free_study(smtp_cmds_[i].extra);
#else
            pcre_free(smtp_cmds_[i].extra);
#endif
        }
    }
}
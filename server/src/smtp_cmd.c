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
            // printf("ERROR: Could not compile '%s': %s\n", smtp_cmds_[i].re,
            //    pcre_error_str);
            for (size_t j = 0; j < i; j++) {
                pcre_free(smtp_cmds_[j].re_compiled);
            }
            return CE_INIT_3RD;
        }
        smtp_cmds_[i].extra =
            pcre_study(smtp_cmds_[i].re_compiled, 0, &pcre_error_str);
        if (pcre_error_str) {
            // printf("ERROR: Could not study '%s': %s\n", smtp_cmds_[i].re,
            //    pcre_error_str);
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

error_code_t smtp_cmd_check(SMTP_CMD cmd, match_info_t* match_info) {
    match_info->cmd = cmd;
    if ((cmd < 0) || (cmd > SMTP_CMD_END_DATA)) {
        return CE_INVALID_ARG;
    }
    match_info->sub_str_count =
        pcre_exec(smtp_cmds_[cmd].re_compiled, smtp_cmds_[cmd].extra,
                  match_info->tested_line, strlen(match_info->tested_line), 0,
                  0, match_info->sub_str, SUB_STR_COUNT);
    if (match_info->sub_str_count < 0) {
        switch (match_info->sub_str_count) {
            case PCRE_ERROR_NOMATCH:
                // printf("String did not match the pattern\n");
                break;
            case PCRE_ERROR_NULL:
                // printf("Something was null\n");
                break;
            case PCRE_ERROR_BADOPTION:
                // printf("A bad option was passed\n");
                break;
            case PCRE_ERROR_BADMAGIC:
                // printf("Magic number bad (compiled re corrupt?)\n");
                break;
            case PCRE_ERROR_UNKNOWN_NODE:
                // printf("Something kooky in the compiled re\n");
                break;
            case PCRE_ERROR_NOMEMORY:
                // printf("Ran out of memory\n");
                break;
            default:
                // printf("Unknown error\n");
                break;
        }
        return CE_COMMON;
    }

    if (match_info->sub_str_count == 0) {
        // printf("But too many substrings were found to fit in "
        //    "subStrVec!\n");
        // Set rc to the max number of substring matches possible.
        match_info->sub_str_count = SUB_STR_COUNT / 3;
    }

    return CE_SUCCESS;
}

error_code_t smtp_cmd_get_substring(match_info_t* match_info, int index,
                                    char* buf, size_t buf_size) {
    pcre_copy_substring(match_info->tested_line, match_info->sub_str,
                        match_info->sub_str_count, index, buf, buf_size);
    return CE_SUCCESS;
}

error_code_t smtp_cmd_get_named_substring(match_info_t* match_info, char* name,
                                          char* buf, size_t buf_size) {
    pcre_copy_named_substring(smtp_cmds_[match_info->cmd].re_compiled,
                              match_info->tested_line, match_info->sub_str,
                              match_info->sub_str_count, name, buf, buf_size);
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
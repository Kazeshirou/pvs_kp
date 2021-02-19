#pragma once

#define SMTP_LOCAL_PART_MAX_SIZE 64
#define SMTP_DOMAIN_MAX_SIZE     255
#define SMTP_PATH_MAX_SIZE       256
#define SMTP_CMD_MAX_SIZE        512
#define SMTP_REPLY_MAX_SIZE      512
#define SMTP_TEXT_LINE_MAX_SIZE  1000
#define SMTP_RCPT_MAX_SIZE       100

#define SMTP_SERVER_TIMEOUT_S 5 * 60

#define SMTP_CR_LF "\r\n"

#define SMTP_OPENING_MSG          "220" SMTP_CR_LF
#define SMTP_SUCCESS_ANSWER       "250 OK" SMTP_CR_LF
#define SMTP_DATA_ACCEPTED_ANSWER "354" SMTP_CR_LF
#define SMTP_QUIT_SUCCESS_ANSWER  "221 OK" SMTP_CR_LF

#define SMTP_VRFY_ANSWER "502" SMTP_CR_LF

#define SMTP_LINE_TOO_LONG_ANSWER      "500 Line too long" SMTP_CR_LF
#define SMTP_PATH_TOO_LONG_ANSWER      "501 Path too long" SMTP_CR_LF
#define SMTP_TOO_MANY_RCPT_ANSWER      "452 Too many recipients" SMTP_CR_LF
#define SMTP_TOO_MUCH_MAIL_DATA_ANSWER "552 Too much mail data" SMTP_CR_LF
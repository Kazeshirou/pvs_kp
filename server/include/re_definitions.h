#pragma once

#define RE_CRLF                 "(\\x0D\\x0A)"
#define RE_LET_DIG              "([a-zA-Z0-9])"
#define RE_LDH_STR              "([a-zA-Z0-9\\-]*" RE_LET_DIG ")"
#define RE_SUB_DOMAIN           "(" RE_LET_DIG RE_LDH_STR "?)"
#define RE_DOMAIN               "(" RE_SUB_DOMAIN "(\\." RE_SUB_DOMAIN ")*)"
#define RE_0_255                "((25[0-5])|(2[0-4][1-9])|([01]?[0-9][0-9]?))"
#define RE_IPV4_ADDRESS_LITERAL "(" RE_0_255 "(." RE_0_255 "){3})"
#define RE_IPV6_ADDRESS_LITERAL \
    "("                         \
    ")"
#define RE_DCONTENT                "([\\x{21}-\\x{5A}\\x{5E}-\\x{7C}])"
#define RE_GENERAL_ADDRESS_LITERAL "(" RE_LDH_STR ":" RE_DCONTENT "+)"
#define RE_ADDRES_LITERAL                                       \
    "(\\[(" RE_IPV4_ADDRESS_LITERAL "|" RE_IPV6_ADDRESS_LITERAL \
    "|" RE_GENERAL_ADDRESS_LITERAL ")])"

#define RE_HELO "^helo " RE_DOMAIN RE_CRLF
#define RE_EHLO "^ehlo (" RE_DOMAIN "|" RE_ADDRES_LITERAL ")" RE_CRLF
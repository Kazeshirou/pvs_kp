#pragma once

#define RE_CRLF                 "(\\x0D\\x0A)"
#define RE_LET_DIG              "[a-zA-Z0-9]"
#define RE_LDH_STR              "([a-zA-Z0-9\\-]*" RE_LET_DIG ")"
#define RE_SUB_DOMAIN           "(" RE_LET_DIG RE_LDH_STR "?)"
#define RE_DOMAIN               "(" RE_SUB_DOMAIN "(\\." RE_SUB_DOMAIN ")*)"
#define RE_0_255                "((25[0-5])|(2[0-4][1-9])|([01]?[0-9][0-9]?))"
#define RE_IPV4_ADDRESS_LITERAL "(" RE_0_255 "(." RE_0_255 "){3})"
#define RE_HEXDIGIT             "[0-9A-Fa-f]"
#define RE_IPV6_HEX             "(" RE_HEXDIGIT "{1,4})"
#define RE_IPV6_HEX_WITH_COLON  "(:" RE_IPV6_HEX ")"
#define RE_IPV6_FULL            "(" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{7})"
#define RE_IPV6_COMP0           "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,5})?::)"
#define RE_IPV6_COMP1 \
    "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,4})?::" RE_IPV6_HEX ")"
#define RE_IPV6_COMP2                              \
    "((" RE_IPV6_HEX        RE_IPV6_HEX_WITH_COLON \
    "{0,3})?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON ")"
#define RE_IPV6_COMP3                              \
    "((" RE_IPV6_HEX        RE_IPV6_HEX_WITH_COLON \
    "{0,2})?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{2})"
#define RE_IPV6_COMP4                          \
    "((" RE_IPV6_HEX    RE_IPV6_HEX_WITH_COLON \
    "?)?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{3})"
#define RE_IPV6_COMP5 \
    "(" RE_IPV6_HEX "?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{4})"
#define RE_IPV6_COMP6 "(::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{5})"
#define RE_IPV6_COMP                                                        \
    "(" RE_IPV6_COMP0 "|" RE_IPV6_COMP1 "|" RE_IPV6_COMP2 "|" RE_IPV6_COMP3 \
    "|" RE_IPV6_COMP4 "|" RE_IPV6_COMP5 "|" RE_IPV6_COMP6 ")"
#define RE_IPV6V4_FULL \
    "(" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{5}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP0                     \
    "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON \
    "{0,3})?::" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP1                                             \
    "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,2})?::" RE_IPV6_HEX \
    ":" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP2                        \
    "((" RE_IPV6_HEX    RE_IPV6_HEX_WITH_COLON \
    "?)?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON ":" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP3                                      \
    "(" RE_IPV6_HEX "?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON \
    "{2}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP4 \
    "(::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{3}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP                                          \
    "(" RE_IPV6V4_COMP0 "|" RE_IPV6V4_COMP1 "|" RE_IPV6V4_COMP2 \
    "|" RE_IPV6V4_COMP3 "|" RE_IPV6V4_COMP4 ")"
#define RE_IPV6_ADDR \
    "(" RE_IPV6_FULL "|" RE_IPV6_COMP "|" RE_IPV6V4_FULL "|" RE_IPV6V4_COMP ")"
#define RE_IPV6_ADDRESS_LITERAL    "(IPv6:" RE_IPV6_ADDR ")"
#define RE_DCONTENT                "([\\x{21}-\\x{5A}\\x{5E}-\\x{7C}])"
#define RE_GENERAL_ADDRESS_LITERAL "(" RE_LDH_STR ":" RE_DCONTENT "+)"
#define RE_ADDRES_LITERAL                                       \
    "(\\[(" RE_IPV4_ADDRESS_LITERAL "|" RE_IPV6_ADDRESS_LITERAL \
    "|" RE_GENERAL_ADDRESS_LITERAL ")])"

#define RE_HELO "^helo " RE_DOMAIN RE_CRLF
#define RE_EHLO "^ehlo (" RE_DOMAIN "|" RE_ADDRES_LITERAL ")" RE_CRLF
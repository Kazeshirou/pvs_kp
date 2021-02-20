#pragma once

// clang-format off
#define RE_CRLF                 "( ?\\x0D\\x0A)"
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
#define RE_IPV6_COMP1 "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,4})?::" RE_IPV6_HEX ")"
#define RE_IPV6_COMP2 "((" RE_IPV6_HEX        RE_IPV6_HEX_WITH_COLON "{0,3})?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON ")"
#define RE_IPV6_COMP3 "((" RE_IPV6_HEX        RE_IPV6_HEX_WITH_COLON "{0,2})?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{2})"
#define RE_IPV6_COMP4 "((" RE_IPV6_HEX    RE_IPV6_HEX_WITH_COLON "?)?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{3})"
#define RE_IPV6_COMP5 "(" RE_IPV6_HEX "?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{4})"
#define RE_IPV6_COMP6 "(::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{5})"
#define RE_IPV6_COMP  "(" RE_IPV6_COMP0 "|" RE_IPV6_COMP1 "|" RE_IPV6_COMP2 "|" RE_IPV6_COMP3 "|" RE_IPV6_COMP4 "|" RE_IPV6_COMP5 "|" RE_IPV6_COMP6 ")"
#define RE_IPV6V4_FULL "(" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{5}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP0 "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,3})?::" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP1 "((" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{0,2})?::" RE_IPV6_HEX ":" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP2 "((" RE_IPV6_HEX    RE_IPV6_HEX_WITH_COLON "?)?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON ":" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP3 "(" RE_IPV6_HEX "?::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{2}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP4 "(::" RE_IPV6_HEX RE_IPV6_HEX_WITH_COLON "{3}:" RE_IPV4_ADDRESS_LITERAL ")"
#define RE_IPV6V4_COMP  "(" RE_IPV6V4_COMP0 "|" RE_IPV6V4_COMP1 "|" RE_IPV6V4_COMP2 "|" RE_IPV6V4_COMP3 "|" RE_IPV6V4_COMP4 ")"
#define RE_IPV6_ADDR "(" RE_IPV6_FULL "|" RE_IPV6_COMP "|" RE_IPV6V4_FULL "|" RE_IPV6V4_COMP ")"
#define RE_IPV6_ADDRESS_LITERAL    "(IPv6:" RE_IPV6_ADDR ")"
#define RE_DCONTENT                "([\\x21-\\x5A\\x5E-\\x7C])"
#define RE_GENERAL_ADDRESS_LITERAL "(" RE_LDH_STR ":" RE_DCONTENT "+)"
#define RE_ADDRES_LITERAL "(\\[(" RE_IPV4_ADDRESS_LITERAL "|" RE_IPV6_ADDRESS_LITERAL "|" RE_GENERAL_ADDRESS_LITERAL ")])"
#define RE_AT_DOMAIN       "(@" RE_DOMAIN ")"
#define RE_A_D_L           "(" RE_AT_DOMAIN "(," RE_AT_DOMAIN ")?)"
#define RE_ATEXT           "[a-zA-Z0-9!#$%&'*+-=/?^_`{}|~]"
#define RE_ATOM            "(" RE_ATEXT "+)"
#define RE_DOT_STRING      "(" RE_ATOM "(." RE_ATOM ")*)"
#define RE_QTEXT_SMTP      "([\\x20\\x21]|[\\x23\\x5b]|[\\x5d\\x7c])"
#define RE_QUOTED_PAIRSMTP "(\\x5c[\\x17-\\x7c])"
#define RE_QCONTENTSMTP    "(" RE_QTEXT_SMTP "|" RE_QUOTED_PAIRSMTP ")"
#define RE_QUOTED_STRING   "(\"" RE_QCONTENTSMTP "*\")"
#define RE_LOCAL_PART      "(" RE_DOT_STRING "|" RE_QUOTED_STRING ")"
#define RE_MAILBOX         "(" RE_LOCAL_PART "@(" RE_DOMAIN "|" RE_ADDRES_LITERAL "))"
#define RE_PATH            "(<(" RE_A_D_L ":)?" RE_MAILBOX ">)"
#define RE_REVERSE_PATH    "(" RE_PATH "|(<>))"
#define RE_ESMTP_KEYWORD   "([a-zA-Z0-9][a-zA-Z0-9\\-]*)"
#define RE_ESMTP_VALUE     "(([\\x21-\\x3c]|[\\x3e-\\x7c])+)"
#define RE_ESMTP_PARAM     "(" RE_ESMTP_KEYWORD "(=" RE_ESMTP_VALUE ")?)"
#define RE_MAIL_PARAMETERS "(" RE_ESMTP_PARAM "( " RE_ESMTP_PARAM ")*)"
#define RE_RCPT_PARAMETERS "(" RE_ESMTP_PARAM "( " RE_ESMTP_PARAM ")*)"
#define RE_FORWARD_PATH    RE_PATH
#define RE_STRING          "(" RE_ATOM "|" RE_QUOTED_STRING ")"

#define RE_END_DATA "^.\\x0D\\x0A$"

#define RE_HELO "^helo " RE_DOMAIN RE_CRLF
#define RE_EHLO "^ehlo (" RE_DOMAIN "|" RE_ADDRES_LITERAL ")" RE_CRLF
#define RE_MAIL "^mail from: ?" RE_REVERSE_PATH "( " RE_MAIL_PARAMETERS ")?" RE_CRLF
#define RE_RCPT "^rcpt to: ?((<Postmaster@" RE_DOMAIN ">)|(<Postmaster>)|" RE_FORWARD_PATH ")( " RE_RCPT_PARAMETERS ")?" RE_CRLF
#define RE_DATA "^data" RE_CRLF
#define RE_RSET "^rset" RE_CRLF
#define RE_QUIT "^quit" RE_CRLF
#define RE_VRFY "^vrfy " RE_STRING RE_CRLF

// clang-format on
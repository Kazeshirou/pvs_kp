#pragma once

#include "mstring.h"

string_t* HELO_command();
string_t* EHLO_command();
string_t* MAILFROM_command(const char *from_addr);
string_t* RCPTTO_command(const char *recipient_addr);
string_t* DATA_command();
string_t* ENDDATA_command();
string_t* RSET_command();
string_t* QUIT_command();


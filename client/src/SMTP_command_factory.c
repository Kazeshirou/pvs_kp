#include "SMTP_command_factory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SMTP_constants.h"

string_t* HELO_command() 
{
    string_t *command = NULL;
    char *command_chr = NULL;
    sprintf(command_chr, "HELO %s\r\n", DOMAIN_NAME);

    command = string_init2(command_chr, strlen(command_chr));
    free(command_chr);
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* EHLO_command() 
{
    string_t *command = NULL;
    char command_chr[256] = { 0 };
    sprintf(command_chr, "EHLO %s\r\n", DOMAIN_NAME);

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* MAILFROM_command(const char *from_addr) 
{
    string_t *command = NULL;
    char command_chr[256] = { 0 };
    sprintf(command_chr, "MAIL FROM: %s\r\n", from_addr);

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* RCPTTO_command(const char *recipient_addr) 
{
    string_t *command = NULL;
    char command_chr[256] = { 0 };
    sprintf(command_chr, "RCPT TO: %s\r\n", recipient_addr);

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* RSET_command()
{
    string_t *command = NULL;
    char *command_chr = "RSET\r\n";

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* DATA_command()
{
    string_t *command = NULL;
    char *command_chr = "DATA\r\n";

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* ENDDATA_command()
{
    string_t *command = NULL;
    char *command_chr = "\r\n.\r\n";

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* QUIT_command() 
{
    string_t *command = NULL;
    char *command_chr = "QUIT\r\n";

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

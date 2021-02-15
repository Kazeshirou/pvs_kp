#include "SMTP_command_factory.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SMTP_constants.h"

string_t* HELO_command() 
{
    string_t *command = NULL;
    char *command_chr = NULL;
    sprintf(command_chr, "HELO %s", DOMAIN_NAME);

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
    char *command_chr = NULL;
    sprintf(command_chr, "EHLO %s", DOMAIN_NAME);

    command = string_init2(command_chr, strlen(command_chr));
    free(command_chr);
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* MAILFROM_command(const char *from_addr) 
{
    string_t *command = NULL;
    char *command_chr = NULL;
    sprintf(command_chr, "MAIL FROM: <%s>", from_addr);

    command = string_init2(command_chr, strlen(command_chr));
    free(command_chr);
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* RCPTTO_command(const char *recipient_addr) 
{
    string_t *command = NULL;
    char *command_chr = NULL;
    sprintf(command_chr, "RCPT TO: <%s>", recipient_addr);

    command = string_init2(command_chr, strlen(command_chr));
    free(command_chr);
    if (!command) 
    {
        return NULL;
    }

    return command;
}

string_t* RSET_command()
{
    string_t *command = NULL;
    char *command_chr = "RSET";

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
    char *command_chr = "DATA";

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
    char *command_chr = ".";

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
    char *command_chr = "QUIT";

    command = string_init2(command_chr, strlen(command_chr));
    if (!command) 
    {
        return NULL;
    }

    return command;
}

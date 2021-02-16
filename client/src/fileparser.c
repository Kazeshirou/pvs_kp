#include "fileparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "errors.h"
#include "SMTP_constants.h"

#define MAX_LINE_LENGTH 1024

queue_t* get_filenames(const char *path_to_dir)
{
    DIR *dir = NULL;
    struct dirent* entry = NULL;
    queue_t *filenames = QUEUE_INIT(string_t, string_copy, string_clear);
    string_t *filename;
    int ret = 0;
    
    dir = opendir(path_to_dir);
    if (dir == NULL) 
    {
        printf("Error in opening %s\n", path_to_dir);
        return NULL;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type != DT_REG)
            continue;
        filename = string_init2(entry->d_name, strlen(entry->d_name));
        ret = queue_push_back(filenames, filename);
        string_clear(filename);
        if (ret != SUCCESS)
        {
            // todo log
            break;
        }
    }

    closedir(dir);

    return filenames;
}

char* get_addr(const string_t *filename, int *type)
{
    int meta_start;
    int filename_len = filename->size;

    for (meta_start = filename_len; 
             meta_start >= 0 && filename->data[meta_start] != SERVER_ADDR_SEP; 
             meta_start--)
        ;

    if (meta_start < 4)
    {
        return NULL;
    }

    meta_start--;
    *type = filename->data[meta_start] - '0';
    meta_start--;

    const size_t max_size = meta_start+1;
    char *addr = (char*) calloc(max_size, sizeof(char));
    if (addr == NULL)
        return NULL;
        
    memcpy(addr, filename->data, meta_start);
    return addr;
}

void free_addr(char *addr)
{
    free(addr);
}

size_t get_recipients_count(const char *line)
{
    size_t count = 0;
    int i;
    for (i = 0; i < strlen(line); i++)
        if (line[i] == TO_SEPARATOR)
            count++;
    return count + 1;
}

char** parse_recipients(const char *line, size_t *count)
{
    char *header = X_DOMAIN_TO;
    char *rcpt = NULL;
    int start = strlen(header);
    int end = strlen(line);
    int rcpt_start = 0;
    int rcpt_len = 0;
    *count = get_recipients_count(line);
    size_t current_rcpt_num = 0;
    int i;
    char **recipients = (char**) malloc(sizeof(char*) * *count);
    for (i = start; i < end; i+=2)
    {
        rcpt_start = i;
        while (line[i] != TO_SEPARATOR && i < end)
           i++;
        rcpt_len = i - rcpt_start;
        if (i == end)
            rcpt_len--;
        rcpt = (char*) malloc(sizeof(char) * rcpt_len);
        memcpy(rcpt, line+rcpt_start, rcpt_len);
        rcpt[rcpt_len] = '\0';
        recipients[current_rcpt_num++] = rcpt;
    }
    return recipients;
}

char* parse_sender(const char *line)
{
    char *header = X_DOMAIN_FROM;
    int start = strlen(header);
    int end = strlen(line);
    int sender_len = end-start-1;
    char *sender = (char*) malloc(sizeof(char) * sender_len);
    memcpy(sender, line+start, sender_len);
    sender[sender_len] = '\0';
    return sender;
}

SMTP_message_t* parse_message(const char *queue_dir, 
                              const string_t *filename)
{
    SMTP_message_t *message = SMTP_message_init();
    if (!message)
    {
        return NULL;
    }
    int line_num = 0;
    char line[MAX_LINE_LENGTH+1] = {0};
    string_t *queue_dir_str = string_init2(queue_dir, strlen(queue_dir));
    string_t *full_filename = concat_with_sep(queue_dir_str, filename, FILENAME_SEP);
    FILE *f = fopen(full_filename->data, "r");
    int max_msg_lines = 64;
    message->msg = (string_t**) malloc(sizeof(string_t*) * max_msg_lines);

    string_clear(queue_dir_str);
    string_clear(full_filename);

    if (f)
    {
        while (fgets(line, MAX_LINE_LENGTH, f)) 
        {
            if (line_num == 0)
                message->recipients_addr = parse_recipients(line, &(message->recipients_count));
            else if (line_num == 1)
                message->from_addr = parse_sender(line);
            else 
            {
                if (max_msg_lines == message->msg_lines)
                {
                    max_msg_lines *= 2;
                    message->msg = (string_t**) realloc(message->msg, sizeof(string_t*) * max_msg_lines);
                }
                message->msg[message->msg_lines++] = string_init2(line, strlen(line));
            }
            line_num++;
        }
        fclose(f);
    }
    else
    {
        printf("ERROR %s", full_filename->data);
    }

    return message;
}
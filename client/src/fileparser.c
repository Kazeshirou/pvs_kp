#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "fileparser.h"

struct queue_t* get_files_names(char *path_to_dir)
{
    DIR *dir = NULL;
    struct dirent* entry = NULL;
    struct queue_t *files_names = queue_init();
    
    dir = opendir(path_to_dir);
    if (dir == NULL) 
    {
        printf("Error in opening %s\n", path_to_dir);
    }
    else
    { 
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;
            char *fullname = (char*) calloc(1, strlen(path_to_dir)+1);
            fullname = strcpy(fullname, path_to_dir);
            fullname = strcat(fullname, FILENAME_SEP_STR);
            fullname = strcat(fullname, entry->d_name);
            if (queue_push_back(files_names, fullname, strlen(fullname)+1) != 0)
            {
                // todo log
                break;
            }
            free(fullname);
        }

        closedir(dir);
    }

    return files_names;
}

char* get_addr(char *filename)
{
    int i, j;
    int success = 0;
    int filename_len = strlen(filename);

    for (j = filename_len; j >= 0 && filename[j] != FILENAME_SEP; j--)
        ;
    j++;

    const size_t max_size = filename_len-j+1;
    char *addr = (char*) calloc(max_size, sizeof(char));
    for (i = 0; j < filename_len && success == 0; i++, j++)
    {
        if (filename[j] != SERVER_ADDR_SEP)
            addr[i] = filename[j];
        else
            success = 1;
    }
    if (success)
    {
        addr[i] = '\0';
    }
    else
    {
        free(addr);
        addr = NULL;
    }
    return addr;
}

void free_addr(char *addr)
{
    free(addr);
}

struct message_t parse_message(char *filename)
{
    struct message_t message;
    char *buf = NULL;
    FILE *f = fopen(filename, "r");
    long fsize;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    buf = malloc(fsize + 1);
    fread(buf, 1, fsize, f);
    fclose(f);

    buf[fsize] = 0;
    
    message.data = buf;
    message.size = fsize;
    return message;
}



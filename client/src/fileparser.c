#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "fileparser.h"

queue_t* get_files_names(const char *path_to_dir)
{
    DIR *dir = NULL;
    struct dirent* entry = NULL;
    queue_t *files_names = QUEUE_INIT(string_t, string_copy, string_clear);
    string_t *filename;
    int ret = 0;
    
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
            filename = string_init2(entry->d_name, strlen(entry->d_name));
            ret = QUEUE_PUSH_BACK(files_names, *filename);
            string_clear(filename);
            if (ret != 0)
            {
                // todo log
                break;
            }
        }

        closedir(dir);
    }

    return files_names;
}

char* get_addr(const char *filename)
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

string_t* parse_message(char *filename)
{
    string_t *message;
    FILE *f = fopen(filename, "r");
    long fsize;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    message = string_init(fsize);
    fread(message->data, 1, fsize, f);
    fclose(f);

    return message;
}



#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include "fileparser.h"

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
    }
    else
    { 
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type != DT_REG)
                continue;
            filename = string_init2(entry->d_name, strlen(entry->d_name));
            ret = QUEUE_PUSH_BACK(filenames, *filename);
            string_clear(filename);
            if (ret != 0)
            {
                // todo log
                break;
            }
        }

        closedir(dir);
    }

    return filenames;
}

char* get_addr(const string_t *filename)
{
    int i, j;
    int success = 0;
    int filename_len = filename->size;

    for (j = filename_len; j >= 0 && filename->data[j] != FILENAME_SEP; j--)
        ;
    j++;

    const size_t max_size = filename_len-j+1;
    char *addr = (char*) calloc(max_size, sizeof(char));
    for (i = 0; j < filename_len && success == 0; i++, j++)
    {
        if (filename->data[j] != SERVER_ADDR_SEP)
            addr[i] = filename->data[j];
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

string_t* parse_message(const char *queue_dir, const string_t *filename)
{
    string_t *message = NULL;
    string_t *queue_dir_str = string_init2(queue_dir, strlen(queue_dir));
    string_t *full_filename = concat_with_sep(queue_dir_str, filename, FILENAME_SEP);
    FILE *f = fopen(full_filename->data, "r");
    long fsize;

    if (f)
    {
        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

        message = string_init(fsize);
        fread(message->data, 1, fsize, f);

        fclose(f);
    }
    else
    {
        printf("ERROR %s", full_filename->data);
    }

    string_clear(queue_dir_str);
    string_clear(full_filename);

    return message;
}



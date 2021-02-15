#include <stdlib.h>
#include <string.h> //memcpy

#include "mstring.h"

string_t* string_init(size_t size)
{
    string_t *str = (string_t*)malloc(sizeof(string_t));
    if (!str)
    {
        return NULL;
    }

    str->data = (char*)malloc(sizeof(char)*size+1);
    if (!str->data)
    {
        free(str);
        return NULL;
    }

    str->data[size] = '\0';
    str->size = size;
    return str;
}

string_t* string_init2(const char *data, size_t size)
{
    string_t *str = string_init(size);
    if (str)
        memcpy(str->data, data, size);
    return str;
}

void string_clear(void *str)
{
    string_t *str1 = (string_t*) str;
    free(str1->data);
    free(str);
}

void* string_copy(const void *str)
{
    string_t *str1 = (string_t*) str;
    return string_init2(str1->data, str1->size);
}

string_t* concat(const string_t *first, const string_t *second)
{
    size_t size = first->size + second->size;
    char *concated = (char*) malloc(sizeof(char)*size);
    if (!concated)
    {
        return NULL;
    }
    memcpy(concated, first->data, first->size); 
    memcpy(concated + first->size, second->data, second->size); 
    return string_init2(concated, size);
}

string_t* concat_with_sep(const string_t *first, const string_t *second, const char sep)
{
    string_t *res = NULL;
    size_t size = first->size + second->size + 1;
    char *concated = (char*) malloc(sizeof(char)*size);
    if (!concated)
    {
        return NULL;
    }
    
    memcpy(concated, first->data, first->size); 
    concated[first->size] = sep; 
    memcpy(concated + first->size + 1, second->data, second->size); 

    res = string_init2(concated, size);
    free(concated);
    return res;
}
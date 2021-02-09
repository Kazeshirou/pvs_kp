#include <stdlib.h>
#include <string.h> //memcpy

#include "mstring.h"

string_t* string_init(size_t size)
{
    string_t *str = (string_t*)malloc(sizeof(string_t));
    str->data = (char*)malloc(sizeof(char)*size);
    str->size = size;
    return str;
}

string_t* string_init2(const char *data, size_t size)
{
    string_t *str = string_init(size);
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
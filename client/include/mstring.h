#pragma once

typedef struct string__t
{
    char *data;
    size_t size;
} string_t;

string_t* string_init(size_t size);
string_t* string_init2(const char *data, size_t size);
void string_clear(void *str);
void* string_copy(const void *str);

string_t* concat(const string_t *first, const string_t *second);
string_t* concat_with_sep(const string_t *first, const string_t *second, const char sep);
#pragma once

#include <stdlib.h>
#include <string.h>

typedef void* (*copy_constructor_t)(const void*);

static inline void *default_copy_constr(const void* src, size_t size)
{
    if (src == NULL)
        return NULL;
    void *dst = malloc(size);
    memcpy(dst, src, size);
    return dst;
}

static inline void *copy(const void* src, size_t size, copy_constructor_t copy_constr)
{
    if (copy_constr != NULL)
        return copy_constr(src);
    return default_copy_constr(src, size);
}
#pragma once

typedef void (*destructor_t)(void*);

static inline void default_destr(void* src)
{
    if (src != NULL)
        free(src);
}

static inline void destruct(void *src, destructor_t destr) 
{
    if (destr)
        destr(src);
    else
        default_destr(src);
}
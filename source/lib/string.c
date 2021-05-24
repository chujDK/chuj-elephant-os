#include "string.h"
#include "debug.h"
#include "global.h"

void memset(void* dst, uint8_t value, size_t size)
{
    ASSERT(dst != NULL);
    uint8_t* dst_ = (uint8_t *)dst;
    while (size--)
    {
        *dst_ = value;
        ++dst_;
    }
    return;
}

void memcpy(void* dst, void* src, size_t size)
{
    ASSERT(dst != NULL && src != NULL);
    uint8_t* dst_ = (uint8_t *)dst;
    uint8_t* src_ = (uint8_t *)src;
    while (size--)
    {
        *dst_++ = *src_++;
    }
}

int memcmp(const void* a, void* b, size_t size)
{
    ASSERT(a != NULL && b != NULL);
    const char* a_ = (const char *) a;
    const char* b_ = (const char *) b;
    while (size--)
    {
        if (*a_ != *b_)
        {
            return *a_ > *b_ ? 1 : -1;
        }
        a_++;
        b_++;
    }
    return 0;
}

char* strcpy(char* dst, const char* src)
{
    ASSERT(dst != NULL && src != NULL);
    char* dst_r = dst;
    while ((*dst++ = *src++));
    return dst_r;
}

size_t strlen(const char* str)
{
    ASSERT(str != NULL);
    const char* str_start = str;
    while (*str++);
    return (str - str_start - 1);
}

int strcmp(const char* a, const char* b)
{
    ASSERT(a != NULL && b != NULL);
    const char* a_ = (const char *) a;
    const char* b_ = (const char *) b;
    while ( *a_ || *b_ )
    {
        if (*a_ != *b_)
        {
            return *a_ > *b_ ? 1 : -1;
        }
        a_++;
        b_++;
    }
    return 0;
}

char* strchr(const char* str, const uint8_t ch)
{
    ASSERT(str != NULL);
    while (*str != 0)
    {
        if (*str == ch)
        {
            return (char *) str;
        }
        str++;
    }
    return NULL;
}

char* strrchr(const char* str, const uint8_t ch)
{
    ASSERT(str != NULL);
    char* last_found = NULL;
    while (*str != 0)
    {
        if (*str == ch)
        {
            return last_found = (char *) str;
        }
        str++;
    }
    return last_found;
}

char* strcat(char* dst,const char* src)
{
    ASSERT(dst != NULL && src != NULL);
    char* str = dst;
    while(*str++);
    
    --str;
    while((*str++ = *src++));
    return dst;
}

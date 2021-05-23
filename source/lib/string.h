#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"

void memset(void* dst, uint8_t value, size_t size);
void memcpy(void* dst, void* src, size_t size);
int memcmp(const void* a, void* b, size_t size);
char* strcpy(char* dst, const char* src);
size_t strlen(const char* str);
int strcmp(const char* a, const char* b);
char* strchr(const char* str, const uint8_t ch);
char* strrchr(const char* str, const uint8_t ch);
char* strcat(char* dst,const char* src);

#endif

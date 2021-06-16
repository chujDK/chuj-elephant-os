#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "global.h"
#include "stdint.h"
#include "memory.h"
#include "syscall-init.h"

enum SYSCALL_NR
{
	SYS_WRITE,
	SYS_GETPID,
	SYS_MALLOC,
	SYS_FREE
};

size_t getpid();
size_t write(char* str);
void* malloc(size_t bytes);
void free(void* p);

#endif
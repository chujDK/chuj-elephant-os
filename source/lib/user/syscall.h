#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "global.h"
#include "stdint.h"
#include "syscall-init.h"

enum SYSCALL_NR
{
	SYS_GETPID
};

size_t getpid();

#endif
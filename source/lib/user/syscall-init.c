#include "syscall-init.h"
#include "syscall.h"
#include "global.h"
#include "thread.h"
#include "print.h"
#include "console.h"
#include "memory.h"

#define syscall_nr 32
typedef void* syscall;
syscall syscall_table[syscall_nr];

size_t sys_getpid()
{
	return GetCurrentThreadPCB()->pid;
}

size_t sys_write(char* str)
{
	return console_putstr(str);
}

void SyscallInit()
{
	sys_putstr("syscall init start..");
	syscall_table[SYS_GETPID] = sys_getpid;
	syscall_table[SYS_WRITE] = sys_write;
	syscall_table[SYS_MALLOC] = sys_malloc;
	syscall_table[SYS_FREE] = sys_free;
	sys_putstr(" done\n");
}

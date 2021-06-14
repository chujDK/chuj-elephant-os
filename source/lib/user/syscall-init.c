#include "syscall-init.h"
#include "syscall.h"
#include "global.h"
#include "thread.h"
#include "print.h"

#define syscall_nr 32
typedef void* syscall;
syscall syscall_table[syscall_nr];

size_t sys_getpid()
{
	return GetCurrentThreadPCB()->pid;
}

void SyscallInit()
{
	sys_putstr("syscall init start..");
	syscall_table[SYS_GETPID] = sys_getpid;
	sys_putstr(" done\n");
}

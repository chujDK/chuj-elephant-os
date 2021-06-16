#include "syscall.h"

#define _syscall0(NUMBER) 			\
({ 									\
	int retval;						\
	__asm__ volatile				\ 
	(								\
		"int $0x80" :				\
		"=a" (retval) :				\
		"a"  (NUMBER) :				\
		"memory"					\
	);								\
	retval;							\
}) 									\

#define _syscall1(NUMBER, ARG1)				\
({											\
	int retval;								\
	__asm__ volatile						\
	(										\
		"int $0x80" :						\
		"=a" (retval) :						\
		"a" (NUMBER), "b" (ARG1) :			\
		"memory"							\
	);										\
	retval;									\
})											\

#define _syscall2(NUMBER, ARG1, ARG2) 		\
({											\
	int retval;								\
	__asm__ volatile						\
	(										\
		"int $0x80" :						\
		"=a" (retval) :						\
		"a" (NUMBER), "b" (ARG1),			\
		"c" (ARG2) :						\
		"memory"							\
	);										\
	retval;									\
})											\

#define _syscall3(NUMBER, ARG1, ARG2, ARG3) \
({											\
	int retval;								\
	__asm__ volatile						\
	(										\
		"int $0x80" :						\
		"=a" (retval) :						\
		"a" (NUMBER), "b" (ARG1),			\
		"c" (ARG2), "d" (ARG3) :			\
		"memory"							\
	);										\
	retval;									\
})											\

size_t getpid()
{
	return _syscall0(SYS_GETPID);
}

size_t write(char* str)
{
	return _syscall1(SYS_WRITE, str);
}

void* malloc(size_t bytes)
{
	return _syscall1(SYS_MALLOC, bytes);
}

void free(void* p)
{
	return _syscall1(SYS_FREE, p);
}

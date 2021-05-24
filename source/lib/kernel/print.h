#ifndef _LIB_KERNEL_PRINT_H
#define _LIB_KERNEL_PRINT_H
#include "print_asm.h"
#include "stdint.h"
int sys_puthex(unsigned int num);
int sys_putint(int num);
extern void sys_putchar(uint8_t char_asci);
extern int sys_putstr(char* str);
#endif

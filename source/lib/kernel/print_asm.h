#ifndef _LIB_KERNEL_PRINT_ASM_H
#define _LIB_KERNEL_PRINT_ASM_H
#include "stdint.h"
void sys_putchar(uint8_t char_asci);
int sys_putstr(char* str);
void sys_setcursor(uint16_t place);
#endif

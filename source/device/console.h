#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H
#include "stdint.h"
#include "print.h"
void ConsoleInit();
int console_putstr(char *str);
void console_putchar(char char_asi);
int console_putint(int num);
int console_puthex(unsigned int num);
#endif
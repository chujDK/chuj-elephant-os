#include "debug.h"
#include "print.h"
#include "interrupt.h"


void PanicSpin(char* filename, int line, const char* func, const char* condition)
{
    DisableInt();
    sys_putstr("\n\n\n***** error! *****\n");
    sys_putstr("filename: ");sys_putstr(filename);sys_putchar('\n');
    sys_putstr("line: ");sys_putint(line);sys_putchar('\n');
    sys_putstr("function: ");sys_putstr((char *) func);sys_putchar('\n');
    sys_putstr("condition: ");sys_putstr((char *) condition);sys_putchar('\n');
    while(1);
}

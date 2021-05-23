#include "debug.h"
#include "print.h"
#include "interrupt.h"

static void sys_putint(int num)
{
    char str_num[12];
    int cnt = 0;
    while(num)
    {
        str_num[cnt++] = num % 10 + '0';
        num /= 10;
    }
    str_num[cnt] = 0;
    for (int i = 0; i < cnt/2; i++)
    {
        int t = str_num[i];
        str_num[i] = str_num[cnt - i - 1];
        str_num[cnt - i - 1] = t;
    }
    sys_putstr(str_num);
}

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

#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"

static struct lock console_lock;

void ConsoleInit()
{
    LockInit(&console_lock);
}

void ConsoleAcquire()
{
    sys_lock_lock(&console_lock);
}

void ConsoleRelease()
{
    sys_lock_unlock(&console_lock);
}

int console_putstr(char *str)
{
    ConsoleAcquire();
    int output_num = sys_putstr(str);
    ConsoleRelease();
    return output_num;
}

void console_putchar(char char_asi)
{
    ConsoleAcquire();
    sys_putchar(char_asi);
    ConsoleRelease();
}

int console_putint(int num)
{
    ConsoleAcquire();
    int output_num = sys_putint(num);
    ConsoleRelease();
    return output_num;
}
#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"

static struct lock console_lock;

void ConsoleInit()
{
    LockInit(&console_lock);
}

void ConsoleLockAcquire()
{
    sys_lock_lock(&console_lock);
}

void ConsoleLockRelease()
{
    sys_lock_unlock(&console_lock);
}

int console_putstr(char *str)
{
    ConsoleLockAcquire();
    int output_num = sys_putstr(str);
    ConsoleLockRelease();
    return output_num;
}

void console_putchar(char char_asi)
{
    ConsoleLockAcquire();
    sys_putchar(char_asi);
    ConsoleLockRelease();
}

int console_putint(int num)
{
    ConsoleLockAcquire();
    int output_num = sys_putint(num);
    ConsoleLockRelease();
    return output_num;
}

int console_puthex(unsigned int num)
{
    ConsoleLockAcquire();
    int output_num = sys_puthex(num);
    ConsoleLockRelease();
    return output_num; 
}
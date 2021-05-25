#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"

int _start()
{
    sys_putstr("this is kernel!\n");
    char a[16];
    InitAll();

    void* addr = kpalloc(5);
    sys_putstr("we got 5 contiguous virtual page, start at: 0x");
    sys_puthex(addr);
    sys_putchar('\n');

    while(1);
    return 0;
}


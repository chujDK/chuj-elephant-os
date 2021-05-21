#include "print.h"
#include "init.h"

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();
    __asm__ volatile ("sti");
    while(1);
    return 0;
}


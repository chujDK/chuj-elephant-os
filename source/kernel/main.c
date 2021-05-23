#include "print.h"
#include "init.h"
#include "debug.h"

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();
    ASSERT(1 == 2);
    while(1);
    return 0;
}


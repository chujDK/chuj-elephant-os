#include "print.h"

int _start()
{
    sys_putstr("this is kernel!\n");
    sys_putstr("Back Space\b");
    while(1);
    return 0;
}


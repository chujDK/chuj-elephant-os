#include "init.h"
#include "print.h"
#include "interrupt.h"

void InitAll()
{
    sys_putstr("init_all..\n");
    IdtInit();
    return;
}

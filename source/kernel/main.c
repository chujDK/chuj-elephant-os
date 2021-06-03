#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"

void KThreadTest(void *arg);

int _start()
{
    sys_putstr("this is kernel!\n");
    char a[16];
    InitAll();

    ThreadStart("KThreadTest", 21, KThreadTest, "argA ");
    ThreadStart("KThreadTest", 6, KThreadTest, "argB ");

    EnableInt();
    while(1)
    {
        sys_putstr("main ");
    }
    return 0;
}

void KThreadTest(void *arg)
{
    char *para = (char *) arg;
    while(1)
    {
        sys_putstr(arg);
    }
}

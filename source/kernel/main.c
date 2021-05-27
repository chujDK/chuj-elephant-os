#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"

void KThreadTest(void *arg);

int _start()
{
    sys_putstr("this is kernel!\n");
    char a[16];
    InitAll();

    ThreadStart("KThreadTest", 31, KThreadTest, "argA ");
    while(1);
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


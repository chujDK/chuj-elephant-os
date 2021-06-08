#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "keyboard.h"
#include "ioqueue.h"

void KThreadTest();

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();

    /* this thread output the input from the keyboard */
    ThreadStart("KThreadTestA", 31, KThreadTest, "");

    EnableInt();

    void* mapped_addr = VirtualAddrMapping(KERNEL_POOL, 0xCC00C000);
    console_putstr("mapping 0x");
    console_puthex(mapped_addr);
    console_putstr(" to 0x");
    console_puthex(VirtualAddrToPhysicAddr(mapped_addr));

    while(1);
    return 0;
}

void KThreadTest()
{
    while(1)
    {
        enum int_status old_statu = DisableInt();
        if (!ioqueueEmpty(&keyboard_IO_buf))
        {
            console_putchar(ioqueue_getchar(&keyboard_IO_buf));   
        }
        SetIntStatus(old_statu);
    }
}
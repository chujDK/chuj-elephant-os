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
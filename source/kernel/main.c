#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "process.h"

void k_thread();
void KeyboardOutput();
void user_thread_a();
void user_thread_b();

int test_var_a = 0, test_var_b = 0;

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();


    /* this thread output the input from the keyboard */
    ThreadStart("keyboard_output", 31, KeyboardOutput, "");

    ThreadStart("display_var", 31, k_thread, "");

    EnableInt();

    ExecProcess(user_thread_a, "userthreada");
    ExecProcess(user_thread_b, "userthreadb");


    while(1);
    return 0;
}

void user_thread_a()
{
    while (1)
    {
        test_var_a++;
    }
}

void user_thread_b()
{
    while (1)
    {
        test_var_b++;
    }
}

void k_thread()
{
    while(0)
    {
        console_putstr("test_var_a: ");
        console_putint(test_var_a);
        console_putchar('\n');
        console_putstr("test_var_b: ");
        console_putint(test_var_b);
        console_putchar('\n');
    }
    while (1)
    {
    }
    
}

void KeyboardOutput()
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
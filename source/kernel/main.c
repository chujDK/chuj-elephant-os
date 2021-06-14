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
#include "syscall.h"

void k_thread();
void KeyboardOutput();
void user_thread_a();
void user_thread_b();

int test_var_a = 0, test_var_b = 0;

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();

    ExecProcess(user_thread_a, "userthreada");
    ExecProcess(user_thread_b, "userthreadb");
    EnableInt();

    /* this thread output the input from the keyboard */
    ThreadStart("keyboard_output", 31, KeyboardOutput, "");
    ThreadStart("display_var", 31, k_thread, "");


    while(1);
    return 0;
}

void user_thread_a()
{
    while (1)
    {
        test_var_a = getpid();
    }
}

void user_thread_b()
{
    while (1)
    {
        test_var_b = getpid();
    }
}

void k_thread()
{
    console_putstr("k_thread pid: ");
    console_putint(sys_getpid());
    console_putchar('\n');
    console_putstr("user_thread_a pid: ");
    console_putint(test_var_a);
    console_putchar('\n');
    console_putstr("user_thread_b pid: ");
    console_putint(test_var_b);
    console_putchar('\n');
    while(1);
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
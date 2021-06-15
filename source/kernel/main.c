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

void k_thread(size_t nbytes);
void KeyboardOutput();
void user_thread_a();
void user_thread_b();
void k_thread_a(void *);
void k_thread_b(void *);
int test_var_a = 0, test_var_b = 0;

int _start()
{
    sys_putstr("this is kernel!\n");
    InitAll();

    //    ExecProcess(user_thread_a, "userthreada");
    //    ExecProcess(user_thread_b, "userthreadb");

    EnableInt();

    /* this thread output the input from the keyboard */
    ThreadStart("keyboard_output", 5, KeyboardOutput, "");
    ThreadStart("k_thread_a", 31, k_thread_a, "I am thread_a");
    ThreadStart("k_thread_b", 31, k_thread_b, "I am thread_b ");
    while (0)
    {
        #define TMP_SUM 4
        void *p[TMP_SUM];
        for (int i = 0; i < TMP_SUM; i++)
        {
            //p[i] = sys_malloc(4000);
            p[i] = kpalloc(2);
        }
        for (int i = 0; i < TMP_SUM; i++)
        {
            console_putstr("\n\n#");
            console_putint(i);
            console_putstr("virtual: 0x");
            console_puthex(p[i]);
            console_putstr(" physic: 0x");
            console_puthex(VirtualAddrToPhysicAddr(p[i]));
        }
        for (int i = 0; i < TMP_SUM; i++)
        {
            pfree(KERNEL_POOL, p[i], 2);
            //sys_free(p[i]);
        }
    }

    for (int aaa = 0; aaa < 0; aaa++)
    {
        console_putchar('\n');
        void *p = kpalloc(4);
        for (int idx = 0; idx < 4; idx++)
        {
            console_putstr("V: 0x");
            console_puthex(p);
            console_putstr(" P: 0x");
            console_puthex(VirtualAddrToPhysicAddr(p));
            console_putchar('\n');
            p += PAGE_SIZE;
        }
        pfree(KERNEL_POOL, p - 4 * PAGE_SIZE, 4);
    }
    
    while (1)
    {
        /* code */
    }
    
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

/* 在线程中运行的函数 */
void k_thread_a(void *arg)
{
    char *para = arg;
    void *addr1;
    void *addr2;
    void *addr3;
    void *addr4;
    void *addr5;
    void *addr6;
    void *addr7;
    console_putstr(" thread_a start\n");
    int max = 1000;
    while (max-- > 0)
    {
        console_putstr(arg);
        console_putint(max);
        console_putchar('\n');
        int size = 128;
        addr1 = sys_malloc(size);
        size *= 2;
        addr2 = sys_malloc(size);
        size *= 2;
        addr3 = sys_malloc(size);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        size *= 2;
        size *= 2;
        size *= 2;
        size *= 2;
        size *= 2;
        size *= 2;
        size *= 2;
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        size *= 2;
        addr7 = sys_malloc(size);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
    }
    console_putstr(" thread_a end\n");
    while (1)
        ;
}

/* 在线程中运行的函数 */
void k_thread_b(void *arg)
{
    char *para = arg;
    void *addr1;
    void *addr2;
    void *addr3;
    void *addr4;
    void *addr5;
    void *addr6;
    void *addr7;
    void *addr8;
    void *addr9;
    int max = 1000;
    console_putstr(" thread_b start\n");
    while (max-- > 0)
    {
        console_putstr(arg);
        console_putint(max);
        console_putchar('\n');
        int size = 9;
        addr1 = sys_malloc(size);
        size *= 2;
        addr2 = sys_malloc(size);
        size *= 2;
        sys_free(addr2);
        addr3 = sys_malloc(size);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        size *= 2;
        addr7 = sys_malloc(size);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr3);
        sys_free(addr4);

        size *= 2;
        size *= 2;
        size *= 2;
        addr1 = sys_malloc(size);
        addr2 = sys_malloc(size);
        addr3 = sys_malloc(size);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        addr7 = sys_malloc(size);
        addr8 = sys_malloc(size);
        addr9 = sys_malloc(size);
        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
        sys_free(addr5);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr8);
        sys_free(addr9);
    }
    console_putstr(" thread_b end\n");
    while (1)
        ;
}

void k_thread(size_t nbytes)
{
    console_putstr("malloc 0x");
    console_puthex(nbytes);
    console_putstr(" bytes, address: 0x");
    void *p = sys_malloc(nbytes);
    console_puthex((size_t)p);
    console_putchar('\n');
    while (1)
        ;
}

void KeyboardOutput()
{
    while (1)
    {
        enum int_status old_statu = DisableInt();
        if (!ioqueueEmpty(&keyboard_IO_buf))
        {
            console_putchar(ioqueue_getchar(&keyboard_IO_buf));
        }
        SetIntStatus(old_statu);
    }
}
#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "print.h"
#include "list.h"
#include "interrupt.h"
#include "debug.h"
#include "switch.h"

#define PAGE_SIZE 4096

PCB *main_thread;
struct list ready_thread_list; 
struct list all_thread_list;
static struct list_elem *thread_tag;

extern void switch_to(PCB *current_thread, PCB *next_thread);

PCB *GetCurrentThreadPCB()
{
    size_t esp;
    __asm__ volatile ("mov %%esp,%0" : "=g" (esp));
    return (PCB*) (esp & 0xFFFFF000);
}

static void KernelThread(thread_func* function, void* func_arg)
{
    EnableInt();
    function(func_arg);
}

void ScheduleThread()
{
    ASSERT(GetIntStatus() == INT_OFF);
    
    PCB* current_thread = GetCurrentThreadPCB();
    if (current_thread->status == TASK_RUNNING)
    {
        ASSERT(!elem_find(&ready_thread_list, &current_thread->general_tag));

        list_append(&ready_thread_list, &current_thread->general_tag);
        current_thread->cpu_ticks_left = current_thread->priority;
        current_thread->status = TASK_READY;
    }
    else
    {
        // noting to do for now
    }

    ASSERT(!list_empty(&ready_thread_list));
    thread_tag = NULL;

    /* get the first READY task, send it to the CPU */
    thread_tag = list_pop(&ready_thread_list);
    PCB* next_thread = elem2entry(PCB, \
                                          general_tag, thread_tag);
    next_thread->status = TASK_RUNNING;
    switch_to(current_thread, next_thread);
}


void ThreadCreate(PCB* pthread, thread_func function, void* func_arg)
{
    /* reserve the INT_stack's space */
    pthread->self_kernel_stack -= sizeof(struct INT_stack);

    /* reserve the thread_stack's space */
    pthread->self_kernel_stack -= sizeof(struct thread_stack);
    struct thread_stack* kernel_thread = (struct thread_stack*)pthread->self_kernel_stack;
    kernel_thread->eip = KernelThread;
    kernel_thread->function = function;
    kernel_thread->func_arg = func_arg;
    kernel_thread->ebp = kernel_thread->ebp = \
    kernel_thread->esi = kernel_thread->edi = 0;
}

/* init a new thread */
static void InitThread(PCB* pthread, char* name, int priority)
{
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);

    if (pthread == main_thread)
    {
        pthread->status = TASK_RUNNING;
    }
    else
    {
        pthread->status = TASK_READY;
    }
    
    pthread->priority = priority;
    pthread->cpu_ticks_left = priority;
    pthread->cpu_ticks_elapsed = 0;
    pthread->PDE_addr = NULL; /* null represent a kernel thread, use the kernel PDE */
    pthread->self_kernel_stack = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);
    pthread->canary = STACK_CANARY;
}

PCB* ThreadStart(char* name, \
                  int priority, \
                  thread_func function, \
                  void* func_arg)
{
    PCB* thread_PCB = kpalloc(1);
    InitThread(thread_PCB, name, priority);
    ThreadCreate(thread_PCB, function, func_arg);

    ASSERT(!elem_find(&ready_thread_list, &thread_PCB->general_tag));
    list_append(&ready_thread_list, &thread_PCB->general_tag);

    ASSERT(!elem_find(&all_thread_list, &thread_PCB->all_list_tag));
    list_append(&all_thread_list, &thread_PCB->all_list_tag);

    // __asm__ volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret": : "g"(thread_PCB->self_kernel_stack) : "memory");
    return thread_PCB;
}

static void CreateMainThread()
{
    main_thread = GetCurrentThreadPCB();
    InitThread(main_thread, "main", 31);

    ASSERT(!elem_find(&all_thread_list, &main_thread->all_list_tag));
    list_append(&all_thread_list, &main_thread->all_list_tag);
}

void BlockThread(enum task_status stat)
{
    ASSERT( stat == TASK_BLOCKD || \
            stat == TASK_WAITING || \
            stat == TASK_HANGING);
    enum int_status old_int_statu = DisableInt();
    PCB* current_thread = GetCurrentThreadPCB();
    current_thread->status = stat;
    ScheduleThread();
    SetIntStatus(old_int_statu);
}

void UnblockThread(PCB* pthread)
{
    enum int_status old_int_statu = DisableInt();
    ASSERT( pthread->status == TASK_BLOCKD || \
            pthread->status == TASK_HANGING || \
            pthread->status == TASK_WAITING);
    if (pthread->status != TASK_READY)
    {
        ASSERT(!elem_find(&ready_thread_list, &pthread->general_tag));
        if (!elem_find(&ready_thread_list, &pthread->general_tag))
        {
            PANIC("blocked thread in ready_list");
        }
        list_push(&ready_thread_list, &pthread->general_tag);
        pthread->status = TASK_READY;
    }
    SetIntStatus(old_int_statu);
}

/* init multi thread environment */
void ThreadInit()
{
    sys_putstr("thread_init..");
    list_init(&ready_thread_list);
    list_init(&all_thread_list);

    CreateMainThread();
    sys_putstr(" done\n");
}

#include "thread.h"
#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "print.h"

#define PAGE_SIZE 4096
#define STACK_CANARY 0x32512332

static void KernelThread(thread_func* function, void* func_arg)
{
    function(func_arg);
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

void InitThread(PCB* pthread, char* name, int priority)
{
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = TASK_RUNNING;
    pthread->priority = priority;
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

    __asm__ volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret": : "g"(thread_PCB->self_kernel_stack) : "memory");
    return thread_PCB;
}

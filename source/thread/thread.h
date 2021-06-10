#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"
#include "list.h"
#include "memory.h"

#define STACK_CANARY 0x32512332
typedef void thread_func(void*);

enum task_status
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKD,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

struct INT_stack
{
    size_t int_vertor_num;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint16_t gs, __gsh;
    uint16_t fs, __fsh;
    uint16_t es, __esh;
    uint16_t ds, __dsh;

    uint32_t err_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp_at_signal;
    uint32_t ss;
};

struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip) (thread_func* func, void *func_arg);

    /* only used at the first time */
    void (*dummy_ret);
    thread_func *function;
    void *func_arg;
};

typedef struct task_struct
{
    uint32_t *self_kernel_stack;
    enum task_status status;
    uint8_t priority; /* how many ticks the thread running per round */
    char name[16];

    uint8_t cpu_ticks_left;
    uint8_t cpu_ticks_elapsed;

    struct list_elem general_tag;
    struct list_elem all_list_tag;

    size_t PDE_addr;
    struct virtual_addr userprog_vaddr;
    size_t canary;
}PCB;

PCB* ThreadStart(char* name, int priority, thread_func function, void* func_arg);
void ScheduleThread();
PCB *GetCurrentThreadPCB();
void ThreadInit();
void BlockThread();
void UnblockThread();
void ThreadCreate(PCB* pthread, thread_func function, void* func_arg);
void InitThread(PCB* pthread, char* name, int priority);

extern struct list ready_thread_list; 
extern struct list all_thread_list;

#endif

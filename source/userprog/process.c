#include "process.h"
#include "thread.h"
#include "interrupt.h"
#include "memory.h"
#include "debug.h"
#include "tss.h"
#include "console.h"
#include "global.h"
#include "string.h"
#include "print.h"

extern void IntExit();

void StartProcess(void* filename)
{
    void* function = filename;
    PCB* current_thread = GetCurrentThreadPCB();
    current_thread->self_kernel_stack += sizeof(struct thread_stack);
    struct INT_stack* process_stack = (struct INT_stack*) current_thread->self_kernel_stack;
    process_stack->edi = process_stack->esi = process_stack->ebp = \
    process_stack->esp_dummy = \
    process_stack->eax = process_stack->ebx = process_stack->ecx = \
    process_stack->edx = \
    process_stack->gs = 0;

    process_stack->ds = process_stack->es = process_stack->fs = SELECTOR_U_DATA;

    process_stack->eip = function;
    //process_stack->cs = SELECTOR_U_CODE;
    process_stack->cs = SELECTOR_U_CODE;
    process_stack->eflags = (EFLAGS_IF_1 | EFLAGS_IOPL_0 | EFLAGS_MBS);
    process_stack->esp_at_signal = VirtualAddrMapping(USER_POOL, USER_STACK3_VADDR) + PAGE_SIZE;

    process_stack->ss = SELECTOR_U_STACK;
    __asm__ volatile ("movl %0, %%esp;\
                       jmp IntExit" : : "g" (process_stack) : "memory");
}

/* this function setup a process's kernel PDE */
size_t CreatPDE()
{
    size_t* PDE_vaddr = kpalloc(1);
    if (PDE_vaddr == NULL)
    {
        console_putstr("CreatePDE failed: kpalloc failed");
        return NULL;
    }
    /* copy kernel PDE */
    memcpy((size_t*)((size_t)PDE_vaddr + 0x300 * 4), \
           (size_t*)(0xFFFFF000 + 0x300 * 4), \
           1024);

    memcpy((size_t*)((size_t)PDE_vaddr), \
           (size_t*)(0xFFFFF000), \
           4);

    /* update PED addr (PED[1023] point to PED it self) */
    size_t new_PDE_physic_addr = VirtualAddrToPhysicAddr((size_t) PDE_vaddr);
    PDE_vaddr[1023] = new_PDE_physic_addr | PG_US_U | PG_RW_RW | PG_P_1;
    return PDE_vaddr;
}

void CreatUserVaddrBitmap(PCB* user_process)
{
    user_process->userprog_vaddr.vaddr_start = USER_VADDR_START;
    size_t bitmap_page_cnt = DIV_ROUND_UP( \
    (0xC0000000 - USER_VADDR_START) / PAGE_SIZE / 8, PAGE_SIZE);
    user_process->userprog_vaddr.vaddr_bitmap.bits = kpalloc(bitmap_page_cnt);
    ASSERT(user_process->userprog_vaddr.vaddr_bitmap.bits != NULL);
    user_process->userprog_vaddr.vaddr_bitmap.bitmap_bytes_len = \
    (0xC0000000 - USER_VADDR_START) / PAGE_SIZE / 8;
    ASSERT(user_process->userprog_vaddr.vaddr_bitmap.bitmap_bytes_len < 4096 * bitmap_page_cnt);

    BitmapInit(&user_process->userprog_vaddr.vaddr_bitmap);
}

void ExecProcess(void* filename, char* name)
{
    PCB* thread = kpalloc(1);
    InitThread(thread, name, DEFAULT_PRIORITY);
    CreatUserVaddrBitmap(thread);
    ThreadCreate(thread, StartProcess, filename);
    thread->PDE_addr = CreatPDE();
    ArenaInit(thread->user_arena_descs);
    ASSERT(thread->PDE_addr != NULL);
    
    enum int_status old_status = GetIntStatus();

    ASSERT(!elem_find(&ready_thread_list, &thread->general_tag));
    list_append(&ready_thread_list, &thread->general_tag);
    ASSERT(!elem_find(&all_thread_list, &thread->all_list_tag));
    list_append(&all_thread_list, &thread->all_list_tag);

    SetIntStatus(old_status);
}

void ActivePDE(PCB* pthread) 
{
    size_t PDE_physic_addr = 0x100000;

    if (pthread->PDE_addr != NULL)
    {
        PDE_physic_addr = VirtualAddrToPhysicAddr(pthread->PDE_addr);
    }

    __asm__ volatile ("mov %0, %%cr3" : : "r" (PDE_physic_addr) : "memory");
}

void ActiveProcess(PCB* pthread)
{
    ASSERT(pthread != NULL);
    ActivePDE(pthread);

    /*
     * (pthread->PDE_addr == NULL) means that this thread is KERNEL thread
     * in that case tss->esp0 needn't be updated
     */
    if (pthread->PDE_addr)
    {
        UpdateTssEsp(pthread);        
    }
}
#include "sync.h"
#include "stdint.h"
#include "interrupt.h"
#include "debug.h"
#include "thread.h"
#include "print.h"

#define NULL 0

void SemaphoreInit(struct semaphore* psema, uint8_t value)
{
    psema->value = value;
    list_init(&psema->waiting_thread_list);
}

void LockInit(struct lock* plock)
{
    plock->holder = NULL;
    plock->holder_repeat_request_sum = 0;
    SemaphoreInit(&plock->semaphore, 1);
}

void SemaphoreDown(struct semaphore* psema)
{
    enum int_status old_int_statu = DisableInt();
    PCB* current_thread = GetCurrentThreadPCB();
    while(psema->value == 0)
    {
        ASSERT(!elem_find(&psema->waiting_thread_list, &current_thread->general_tag));
        if (elem_find(&psema->waiting_thread_list, &current_thread->general_tag))
        {
            PANIC("thread already in wating list");
        }
        list_append(&psema->waiting_thread_list, &current_thread->general_tag);
        BlockThread(TASK_BLOCKD);
    }
    psema->value--;
    SetIntStatus(old_int_statu);
}

void SemaphoreUp(struct semaphore* psema)
{
    enum int_status old_int_statu = DisableInt();
    if (!list_empty(&psema->waiting_thread_list))
    {
        PCB* thread_blocked = elem2entry(PCB, general_tag, \
        list_pop(&psema->waiting_thread_list));
        UnblockThread(thread_blocked); 
    }
    psema->value++;
    SetIntStatus(old_int_statu);
}

void sys_lock_lock(struct lock *plock)
{
    if (plock->holder != GetCurrentThreadPCB())
    {
        SemaphoreDown(&plock->semaphore); 
        plock->holder = GetCurrentThreadPCB();
        ASSERT(plock->holder_repeat_request_sum == 0);
        plock->holder_repeat_request_sum = 1;
    }
    else
    {
        plock->holder_repeat_request_sum++;
    }
}

void sys_lock_unlock(struct lock* plock)
{
    ASSERT(plock->holder == GetCurrentThreadPCB());
    if (plock->holder_repeat_request_sum > 1)
    {
        plock->holder_repeat_request_sum--;
        return;
    }
    ASSERT(plock->holder_repeat_request_sum == 1)
    plock->holder_repeat_request_sum = 0;
    plock->holder = NULL;
    SemaphoreUp(&plock->semaphore);
}
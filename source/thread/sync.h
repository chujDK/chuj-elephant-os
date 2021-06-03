#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"

struct semaphore
{
    uint8_t value;
    struct list waiting_thread_list;
};

struct lock
{
    PCB* holder;
    struct semaphore semaphore;
    uint32_t holder_repeat_request_sum; /* repeat times of the lock's owner's requests */
};

#endif
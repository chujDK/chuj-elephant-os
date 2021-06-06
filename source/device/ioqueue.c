#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

void ioqueueInit(struct ioqueue* queue)
{
    LockInit(&queue->lock);
    queue->sleeping_consumer = NULL;
    queue->sleeping_producer = NULL;
    queue->head = queue->tail = 0;
}

static size_t ptr_next_pos(size_t idx)
{
    return (++idx) % bufsize;
}

uint8_t ioqueueFull(struct ioqueue* queue)
{
    ASSERT(GetIntStatus() == INT_OFF);
    return (ptr_next_pos(queue->head) == queue->tail);
}

uint8_t ioqueueEmpty(struct ioqueue* queue)
{
    ASSERT(GetIntStatus() == INT_OFF);
    return (queue->head == queue->tail); 
}

/* make current thread (maybe producer or consumer) block itself */
static void ioqueueBlock(PCB** waiter)
{
    ASSERT(waiter != NULL && *waiter == NULL);
    *waiter = GetCurrentThreadPCB();
    BlockThread(TASK_BLOCKD);
}

/* wake up a blocked thread */
static void ioqueueWakeup(PCB** waiter)
{
    ASSERT(*waiter != NULL);
    UnblockThread(*waiter);
    *waiter = NULL;
}

char ioqueue_getchar(struct ioqueue* queue)
{
    ASSERT(GetIntStatus() == INT_OFF);

    while (ioqueueEmpty(queue))
    {
        sys_lock_lock(&queue->lock);
        /* make current thread (consumer) blocked, and record it */
        ioqueueBlock(&queue->sleeping_consumer);
        sys_lock_unlock(&queue->lock);
    }
    
    char byte = queue->buf[queue->tail];
    queue->tail = ptr_next_pos(queue->tail);

    if (queue->sleeping_producer != NULL)
    {
        ioqueueWakeup(&queue->sleeping_producer);
    }

    return byte;
}

void ioqueue_putchar(struct ioqueue* queue, char byte)
{
    ASSERT(GetIntStatus() == INT_OFF);

    while (ioqueueFull(queue))
    {
        sys_lock_lock(&queue->lock);
        /* make current thread (producer) blocked, and record it */
        ioqueueBlock(&queue->sleeping_producer);
        sys_lock_unlock(&queue->lock);
    }

    queue->buf[queue->head] = byte;
    queue->head = ptr_next_pos(queue->head);
    
    if (queue->sleeping_consumer != NULL)
    {
        ioqueueWakeup(&queue->sleeping_producer);
    }
    return;
}
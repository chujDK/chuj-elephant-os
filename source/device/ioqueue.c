#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

void IOqueueInit(struct ioqueue* queue)
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

uint8_t IOqueueFull(struct ioqueue* queue)
{
    ASSERT(GetIntStatus() == INT_OFF);
    return (ptr_next_pos(queue->head) == queue->tail);
}
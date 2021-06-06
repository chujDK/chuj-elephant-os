#ifndef _DEVICE_IOQUEUE_H
#define _DEVICE_IOQUEUE_H
#include "global.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64

struct ioqueue
{
    struct lock lock;

    PCB* sleeping_producer;
    PCB* sleeping_consumer;
    char buf[bufsize];
    size_t head;
    size_t tail;
};

/* get one byte from the buf */
char ioqueue_getchar(struct ioqueue* queue);

/* put one byte to the buf */
void ioqueue_putchar(struct ioqueue* queue, char byte);

void ioqueueInit(struct ioqueue* queue);

uint8_t ioqueueFull(struct ioqueue* queue);

uint8_t ioqueueEmpty(struct ioqueue* queue);

#endif
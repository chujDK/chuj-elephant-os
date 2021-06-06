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

#endif
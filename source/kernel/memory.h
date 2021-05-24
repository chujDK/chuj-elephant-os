#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

/* manage virtual addr mapping */
struct virtual_addr
{
    struct bitmap vaddr_bitmap;
    size_t vaddr_start;
};

extern struct memory_pool kernel_memory_pool, user_memory_pool;
void VmemInit();
#endif

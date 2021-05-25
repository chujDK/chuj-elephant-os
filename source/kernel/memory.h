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

enum pool_flags
{
    KERNEL_POOL = 1,
    USER_POOL = 2
};

/* Present Bit(in or out of physic memory) */
#define PAGE_P_1 1
#define PAGE_P_0 0

#define PAGE_RW_R 0     /* R-* */
#define PAGE_RW_RW 2    /* RW* */

#define PAGE_US_S 0     /* System */
#define PAGE_US_U 4     /* User */

extern struct memory_pool kernel_memory_pool, user_memory_pool;
void VmemInit();
void* kpalloc(size_t page_cnt);

#endif

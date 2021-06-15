#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"

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

/* memory alloc */
#define ARENA_SUM 7
struct memory_block
{
    struct list_elem free_elem;
};

struct arena_desc
{
    size_t block_size;
    size_t memory_blocks_sum;
    struct list free_list;
};

struct arena
{
    struct arena_desc* desc;
    /*
     * large_request == 1 => cnt = num of alloced pages
     * large_request == 0 => cnt = num of free memory blocks
     */
    size_t cnt;
    int large_request;
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
void* upalloc(size_t page_cnt);
size_t VirtualAddrToPhysicAddr(size_t vaddr);
void* VirtualAddrMapping(enum pool_flags pf, size_t vaddr);
void ArenaInit(struct arena_desc* desc_array);
void* sys_malloc(size_t size);
void pfree(enum pool_flags pf, size_t v_addr, size_t page_cnt);
void sys_free(void* p);

void* GetVirtualPage(enum pool_flags pf, size_t request_page_cnt);
void ReturnVirtualPage(enum pool_flags pf, size_t v_addr, size_t page_cnt);

#endif

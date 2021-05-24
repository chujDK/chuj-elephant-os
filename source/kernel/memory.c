#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"

#define PAGE_SIZE 4096

#define MEM_BITMAP_BASE 0xC0009A00

#define KERNEL_HEAP_BASE 0xC0100000

struct memory_pool
{
    struct bitmap pool_bitmap;
    size_t physic_addr_start;
    size_t pool_size;
};

struct memory_pool kernel_memory_pool, user_memory_pool;
struct virtual_addr kernel_vaddr;

static void VmemPoolInit(size_t all_mem)
{
    sys_putstr("    memory_pool_init start..");
    size_t page_table_size = PAGE_SIZE * 256;
    size_t used_mem = page_table_size + 0x100000;
    size_t free_mem = all_mem - used_mem;
    size_t free_pages_sum = free_mem / PAGE_SIZE;
    size_t kernel_free_pages = free_pages_sum / 2;
    size_t user_free_pages = free_pages_sum - kernel_free_pages;

    size_t kernel_bitmap_length = kernel_free_pages / 8;
    size_t user_bitmap_length = user_free_pages / 8;

    size_t kernel_memory_pool_start = used_mem;
    size_t user_memory_pool_start = kernel_memory_pool_start + kernel_free_pages * PAGE_SIZE;

    kernel_memory_pool.pool_bitmap.bitmap_bytes_len = kernel_bitmap_length;
    user_memory_pool.pool_bitmap.bitmap_bytes_len = user_bitmap_length;
    kernel_memory_pool.pool_bitmap.bits = (void *) MEM_BITMAP_BASE;
    user_memory_pool.pool_bitmap.bits = (void *) (MEM_BITMAP_BASE + kernel_bitmap_length);

    kernel_memory_pool.physic_addr_start = kernel_memory_pool_start;
    user_memory_pool.physic_addr_start = user_memory_pool_start;

    kernel_memory_pool.pool_size = kernel_free_pages * PAGE_SIZE;
    user_memory_pool.pool_size = user_free_pages * PAGE_SIZE;
    
    sys_putstr("\n        kernel_pool_bitmap_start: 0x");
    sys_puthex((int)kernel_memory_pool.pool_bitmap.bits);
    sys_putstr("\n        kernel_pool_physic_addr_start: 0x");
    sys_puthex(kernel_memory_pool.physic_addr_start);

    sys_putstr("\n        user_pool_bitmap_start: 0x");
    sys_puthex((int)user_memory_pool.pool_bitmap.bits);
    sys_putstr("\n        user_pool_physic_addr_start: 0x");
    sys_puthex(user_memory_pool.physic_addr_start);

    BitmapInit(&kernel_memory_pool.pool_bitmap);
    BitmapInit(&user_memory_pool.pool_bitmap);  

    kernel_vaddr.vaddr_bitmap.bitmap_bytes_len = kernel_bitmap_length;
    kernel_vaddr.vaddr_bitmap.bits = \
        (void*)(kernel_bitmap_length + user_bitmap_length + MEM_BITMAP_BASE);
    kernel_vaddr.vaddr_start = KERNEL_HEAP_BASE;
    BitmapInit(&kernel_vaddr.vaddr_bitmap);   
    sys_putstr("\n    done\n");
}

void VmemInit()
{
    sys_putstr("vmem_init start..\n");
    size_t memory_total_bytes = *(size_t *) (0x810);
    VmemPoolInit(memory_total_bytes);
    sys_putstr("done\n");
}

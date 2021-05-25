#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "string.h"
#include "debug.h"

#define PAGE_SIZE 4096

#define MEM_BITMAP_BASE 0xC0009A00

#define KERNEL_HEAP_BASE 0xC0100000

#define PTE_IDX(addr) (((size_t)(addr) & 0x003FF000) >> 12)
#define PDE_IDX(addr) (((size_t)(addr) & 0xFFC00000) >> 22)

struct memory_pool
{
    struct bitmap pool_bitmap;
    size_t physic_addr_start;
    size_t pool_size;
};

struct memory_pool kernel_memory_pool, user_memory_pool;
struct virtual_addr kernel_vaddr;

/* get request_page_cnt pages, they must be contiguous */
static void* GetVirtualPage(enum pool_flags pf, size_t request_page_cnt)
{
    size_t Vaddr_start = NULL;
    size_t physic_page_bit_idx = -1;
    size_t cnt = 0;
    if (pf == KERNEL_POOL)
    {
        if(( physic_page_bit_idx = BitmapScan(&kernel_memory_pool.pool_bitmap, request_page_cnt) ) != -1)
        {
            while (cnt < request_page_cnt)
            {
                BitmapSetBit(&kernel_memory_pool.pool_bitmap, physic_page_bit_idx + cnt++, 1); /* set this page is mapped */
            }
            Vaddr_start = kernel_vaddr.vaddr_start + physic_page_bit_idx * PAGE_SIZE;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        /* user pool, unused for now */
    }
    return (void *)Vaddr_start;
}

void* GetPTEPointer(void* vaddr)
{
    void* pte = (void *) (0xFFC00000 + \
        (((size_t)vaddr & 0xFFC00000) >> 10) + \
        PTE_IDX(vaddr) * 4);
    return pte;
}

void* GetPDEPointer(void* vaddr)
{
    void* pde = (void *) (0xFFFFF000 + PDE_IDX(vaddr) * 4);
    return pde;
}

/* alloc one page from the m_pool */
static void* AllocOnePhysicPage(struct memory_pool* m_pool)
{
    int bit_idx = BitmapScan(&m_pool->pool_bitmap, 1); 
    if (bit_idx == -1)
    {
        return NULL;
    }
    else
    {
        BitmapSetBit(&m_pool->pool_bitmap, bit_idx, 1);
        size_t page_physic_addr = ((bit_idx * PAGE_SIZE) + m_pool->physic_addr_start);
        return (void *) page_physic_addr;
    }
}

static void PageMapping(void* v_addr, void* physic_page_addr)
{
    void* pde_addr = GetPDEPointer(v_addr);
    void* pte_addr = GetPTEPointer(v_addr);
    
    if ((*(size_t*)pde_addr) & PAGE_P_1) /* this PTE is existed */
    {
        /* mapping to the same physic_page */
        ASSERT(!((*(size_t*)pte_addr) & PAGE_P_1));
        if (!((*(size_t*)pte_addr) & PAGE_P_1))
        {
            *(size_t*)pte_addr = ((size_t)physic_page_addr | PAGE_P_1 | PAGE_US_U | PAGE_RW_RW);
        }
        else
        {
            PANIC("MAPPING THE SAME PHYSIC PAGE!!");
            while(1);
        }
    }
    else
    {
        size_t pte_physic_addr = (size_t)(AllocOnePhysicPage(&kernel_memory_pool));
        memset((void*)pte_physic_addr, 0, PAGE_SIZE); /* init the PTE(means nothing mapped) */
        *(size_t*)pde_addr = (pte_physic_addr | PAGE_P_1 | PAGE_US_U | PAGE_RW_RW);
        *(size_t*)pte_addr = ((size_t)physic_page_addr | PAGE_P_1 | PAGE_US_U | PAGE_RW_RW);
    }
}

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

    /* virtual_addr init */
    kernel_vaddr.vaddr_bitmap.bitmap_bytes_len = kernel_bitmap_length;
    kernel_vaddr.vaddr_bitmap.bits = \
        (void*)(kernel_bitmap_length + user_bitmap_length + MEM_BITMAP_BASE);
    kernel_vaddr.vaddr_start = KERNEL_HEAP_BASE;
    BitmapInit(&kernel_vaddr.vaddr_bitmap);   
    
    sys_putstr("\n        kernel_pool_bitmap_start:         0x");
    sys_puthex((int)kernel_memory_pool.pool_bitmap.bits);
    sys_putstr("\n        kernel_pool_physic_addr_start:    0x");
    sys_puthex(kernel_memory_pool.physic_addr_start);
    sys_putstr("\n        kernel_virutal_addr_start:        0x");
    sys_puthex(kernel_vaddr.vaddr_start);

    sys_putstr("\n        user_pool_bitmap_start:           0x");
    sys_puthex((int)user_memory_pool.pool_bitmap.bits);
    sys_putstr("\n        user_pool_physic_addr_start:      0x");
    sys_puthex(user_memory_pool.physic_addr_start);

    BitmapInit(&kernel_memory_pool.pool_bitmap);
    BitmapInit(&user_memory_pool.pool_bitmap);  

    sys_putstr("\n    done\n");
}

void* palloc(enum pool_flags pf, size_t page_cnt)
{
    ASSERT(page_cnt < 3840);/* must less then 15M */
    void* vaddr_start = GetVirtualPage(pf, page_cnt);
    if (vaddr_start == NULL)
    {
        return vaddr_start;
    }

    struct memory_pool* m_pool = (pf == KERNEL_POOL ? &kernel_memory_pool : &user_memory_pool);
    void* page_physic_addr;
    void* vaddr = vaddr_start;
    while(page_cnt--)
    {
        page_physic_addr = AllocOnePhysicPage(m_pool);
        if (page_physic_addr == NULL)
        {
            /* free all alloced page to system, we will fill there when we finished free-related function */
            return NULL;
        }
        PageMapping(vaddr, page_physic_addr);
        vaddr += PAGE_SIZE;
    }

    return vaddr_start;
}

void* kpalloc(size_t page_cnt)
{
    void* vaddr = palloc(KERNEL_POOL, page_cnt);
    if(vaddr != NULL)
    {
        memset(vaddr, 0, page_cnt * PAGE_SIZE);
    }
    return vaddr;
}

void VmemInit()
{
    sys_putstr("vmem_init start..\n");
    size_t memory_total_bytes = *(size_t *) (0x810);
    /* memory_pool init */
    VmemPoolInit(memory_total_bytes);

    sys_putstr("done\n");
}

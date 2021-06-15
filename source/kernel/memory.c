#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "string.h"
#include "debug.h"
#include "thread.h"
#include "sync.h"
#include "list.h"
#include "console.h"
#include "interrupt.h"

#define PAGE_SIZE 4096

#define MEM_BITMAP_BASE 0xC009A000

#define KERNEL_HEAP_BASE 0xC0100000

#define PTE_IDX(addr) (((size_t)(addr) & 0x003FF000) >> 12)
#define PDE_IDX(addr) (((size_t)(addr) & 0xFFC00000) >> 22)

struct memory_pool
{
    struct bitmap pool_bitmap;
    size_t physic_addr_start;
    size_t pool_size;
    struct lock lock;
};

struct memory_pool kernel_memory_pool, user_memory_pool;
struct virtual_addr kernel_vaddr;

struct arena_desc kernel_arena_descs[ARENA_SUM];

/* get request_page_cnt pages, they must be contiguous */
void* GetVirtualPage(enum pool_flags pf, size_t request_page_cnt)
{
    size_t Vaddr_start = NULL;
    size_t virtual_page_bit_idx = -1;
    size_t cnt = 0;
    if (pf == KERNEL_POOL)
    {
        /* kernel vaddr */
        if(( virtual_page_bit_idx = BitmapScan(&kernel_vaddr.vaddr_bitmap, request_page_cnt) ) != -1)
        {
            while (cnt < request_page_cnt)
            {
                BitmapSetBit(&kernel_vaddr.vaddr_bitmap, virtual_page_bit_idx + cnt++, 1); /* set this page is mapped */
            }
            Vaddr_start = kernel_vaddr.vaddr_start + virtual_page_bit_idx * PAGE_SIZE;
        }
        else
        {
            /* haven't set bitmap, so can directly return NULL */
            return NULL;
        }
    }
    else
    {
        /* user vaddr */
        PCB* current_thread = GetCurrentThreadPCB();
        if(( virtual_page_bit_idx = BitmapScan(&current_thread->userprog_vaddr.vaddr_bitmap, request_page_cnt) ) != -1)
        {
            while (cnt < request_page_cnt)
            {
                BitmapSetBit(&current_thread->userprog_vaddr.vaddr_bitmap, virtual_page_bit_idx + cnt++, 1);
            }
            Vaddr_start = (size_t)&current_thread->userprog_vaddr.vaddr_start + virtual_page_bit_idx * PAGE_SIZE;
        }
        else
        {
            return NULL;
        }
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
static void* GetOnePhysicPage(struct memory_pool* m_pool)
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

/* free one physic page to the pool */
void ReturnOnePhysicPage(size_t page_physic_addr)
{
    struct memory_pool* memory_pool;
    size_t bit_idx = 0;
    if (page_physic_addr > user_memory_pool.physic_addr_start)
    {
        memory_pool = &user_memory_pool;
        bit_idx = (page_physic_addr - user_memory_pool.physic_addr_start) / PAGE_SIZE;
    }
    else
    {
        memory_pool = &kernel_memory_pool;
        bit_idx = (page_physic_addr - kernel_memory_pool.physic_addr_start) / PAGE_SIZE;
    }
    BitmapSetBit(&memory_pool->pool_bitmap, bit_idx, 0);
}

/* map a virtual page to a physic page */
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
        size_t pte_physic_addr = (size_t)(GetOnePhysicPage(&kernel_memory_pool));
        *(size_t*)pde_addr = (pte_physic_addr | PAGE_P_1 | PAGE_US_U | PAGE_RW_RW); /* setup mapping first */
        memset((void*)((size_t)pte_addr & 0xFFFFF000), 0, PAGE_SIZE); /* init the PTE(means nothing mapped) */
        *(size_t*)pte_addr = ((size_t)physic_page_addr | PAGE_P_1 | PAGE_US_U | PAGE_RW_RW);
    }
}

/* unmap a virtual page */
void PageUnmapping(size_t v_addr)
{
    size_t* pte = GetPTEPointer(v_addr);
    *pte &= PAGE_P_0;
    __asm__ volatile ("invlpg %0" : : "m" (v_addr) : "memory");
    /* refresh the tlb (cache of paging) */
}

/* return page_cnt vpages to vaddr_pool */
void ReturnVirtualPage(enum pool_flags pf, size_t v_addr, size_t page_cnt)
{
    size_t bit_idx_start = 0, v_addr_start = v_addr, cnt = 0;
    if (pf == KERNEL_POOL)
    {
        bit_idx_start = (v_addr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
        while (cnt < page_cnt)
        {
            BitmapSetBit(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt, 0);
            cnt++;
        }
    }
    else
    {
        PCB* current_thread = GetCurrentThreadPCB();
        bit_idx_start = (v_addr - current_thread->userprog_vaddr.vaddr_start) / PAGE_SIZE;
        while (cnt < page_cnt)
        {
            BitmapSetBit(&current_thread->userprog_vaddr.vaddr_bitmap, \
            bit_idx_start + cnt, 0);
            cnt++;
        }
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
        page_physic_addr = GetOnePhysicPage(m_pool);
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

void pfree(enum pool_flags pf, size_t v_addr_start, size_t page_cnt)
{
    size_t page_physic_addr;
    size_t v_addr = v_addr_start, cnt = 0;
    ASSERT(page_cnt >= 1 && v_addr % PAGE_SIZE == 0);
    page_physic_addr = VirtualAddrToPhysicAddr(v_addr);

    ASSERT(((page_physic_addr % PAGE_SIZE) == 0) && page_physic_addr > 0x102000);
    
    if (page_physic_addr >= user_memory_pool.physic_addr_start)
    {
        v_addr -= PAGE_SIZE;
        while (cnt < page_cnt)
        {
            v_addr += PAGE_SIZE;
            page_physic_addr = VirtualAddrToPhysicAddr(v_addr); 
            ASSERT(page_physic_addr >= user_memory_pool.physic_addr_start);
            ReturnOnePhysicPage(page_physic_addr);
            PageUnmapping(v_addr);
            cnt++;
        }
        ReturnVirtualPage(pf, v_addr_start, page_cnt);
    }
    else
    {
        v_addr -= PAGE_SIZE;
        while (cnt < page_cnt)
        {
            v_addr += PAGE_SIZE;
            page_physic_addr = VirtualAddrToPhysicAddr(v_addr); 
            ASSERT(page_physic_addr < user_memory_pool.physic_addr_start);
            ReturnOnePhysicPage(page_physic_addr);
            PageUnmapping(v_addr);
            cnt++;
        }
        ReturnVirtualPage(pf, v_addr_start, page_cnt);
    }
}

/* return a virtual page that mapped to a kernel page */
void* kpalloc(size_t page_cnt)
{
    sys_lock_lock(&kernel_memory_pool.lock);
    void* vaddr = palloc(KERNEL_POOL, page_cnt);
    sys_lock_unlock(&kernel_memory_pool.lock);
    if(vaddr != NULL)
    {
        memset(vaddr, 0, page_cnt * PAGE_SIZE);
    }
    return vaddr;
}

void* upalloc(size_t page_cnt)
{
    sys_lock_lock(&user_memory_pool.lock);
    void* vaddr = palloc(USER_POOL, page_cnt);
    sys_lock_unlock(&user_memory_pool.lock);
    if(vaddr != NULL)
    {
        memset(vaddr, 0, page_cnt * PAGE_SIZE);
    }
    return vaddr;
}

/* alloc a physic page, and mmapping a virtual page to it */
void* VirtualAddrMapping(enum pool_flags pf, size_t vaddr)
{
    struct memory_pool* memory_pool = pf & KERNEL_POOL ? \
    &kernel_memory_pool : &user_memory_pool;

    sys_lock_lock(&memory_pool->lock);
    
    PCB* curren_thread = GetCurrentThreadPCB();
    size_t bitmap_idx = -1;

    if(curren_thread->PDE_addr != NULL && pf == USER_POOL)
    {
        bitmap_idx = (vaddr - curren_thread->userprog_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bitmap_idx > 0);
        BitmapSetBit(&curren_thread->userprog_vaddr.vaddr_bitmap, bitmap_idx, 1);
    }
    else if(curren_thread->PDE_addr == NULL && pf == KERNEL_POOL)
    {
        bitmap_idx = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bitmap_idx > 0);
        BitmapSetBit(&kernel_vaddr.vaddr_bitmap, bitmap_idx, 1);
    }
    else
    {
        PANIC("VirtualAddrMapping: kernel alloced userspace or user alloced kernel space");
    }

    void* physic_page_addr = GetOnePhysicPage(memory_pool);
    if(physic_page_addr == NULL)
    {
        return NULL;
    }
    PageMapping((void*)vaddr, physic_page_addr);
    sys_lock_unlock(&memory_pool->lock);
    return (void*)vaddr;
}

size_t VirtualAddrToPhysicAddr(size_t vaddr)
{
    size_t* pte = GetPTEPointer(vaddr);
    return (((*pte) & 0xFFFFF000) + (vaddr & 0x00000FFF));
}

void ArenaInit(struct arena_desc* desc_array)
{
    size_t block_size = 16;
    for (int desc_idx = 0; desc_idx < ARENA_SUM; desc_idx++)
    {
        desc_array[desc_idx].block_size = block_size;
        desc_array[desc_idx].memory_blocks_sum = (PAGE_SIZE - sizeof(struct arena)) / block_size;
        list_init(&desc_array[desc_idx].free_list);
        block_size *= 2;
    }
}

void VmemInit()
{
    sys_putstr("vmem_init start..\n");
    size_t memory_total_bytes = *(size_t *) (0x810);
    /* memory_pool init */
    VmemPoolInit(memory_total_bytes);

    /* lock init */
    LockInit(&kernel_memory_pool.lock);
    LockInit(&user_memory_pool.lock);

    /* memory arena init */
    ArenaInit(kernel_arena_descs);

    sys_putstr("done\n");
}

static struct memory_block* arena2block(struct arena* arena, size_t idx)
{
    return (struct memory_block*) ((size_t) arena + sizeof(struct arena) + idx * arena->desc->block_size);
};

static struct arena* block2arena(struct memory_block* block)
{
    return (struct arena*) (((size_t) block) & 0xFFFFF000);
};

void* sys_malloc(size_t size)
{
    enum pool_flags PF;
    struct memory_pool* memory_pool;
    size_t pool_size;
    struct arena_desc* arenas;
    PCB* current_thread = GetCurrentThreadPCB();

    if (current_thread->PDE_addr == NULL)
    {
        PF = KERNEL_POOL;
        pool_size = kernel_memory_pool.pool_size;
        memory_pool = &kernel_memory_pool;
        arenas = kernel_arena_descs; 
    }
    else
    {
        PF = USER_POOL;
        pool_size = user_memory_pool.pool_size;
        memory_pool = &user_memory_pool;
        arenas = current_thread->user_arena_descs;
    }

    if (!(size > 0 && size < pool_size))
    {
        return NULL;
    }

    struct arena* av;
    struct memory_block* p;
    sys_lock_lock(&memory_pool->lock);

    if (size > 1024)
    {
        /* alloc pages directly */
        size_t pages_needed = DIV_ROUND_UP(size + sizeof(struct arena), PAGE_SIZE);
        av = palloc(PF, pages_needed);
        if (av != NULL)
        {
            memset(av, 0, pages_needed * PAGE_SIZE);

            av->desc = NULL;
            av->cnt = pages_needed;
            av->large_request = 1;    
            sys_lock_unlock(&memory_pool->lock);
            return (void*)(av + 1);
        }
        else
        {
            sys_lock_unlock(&memory_pool->lock);
            return NULL;
        }
    }
    else
    {
        size_t arena_idx;
        for (arena_idx = 0; arena_idx < ARENA_SUM; arena_idx++)
        {
            if (size <= arenas[arena_idx].block_size)
            {
                break;
            }
        }

        /* create a arena if there is no free arena */
        if (list_empty(&arenas[arena_idx].free_list))
        {
            av = palloc(PF, 1);
            if (av == NULL)
            {
                sys_lock_unlock(&memory_pool->lock);
                return NULL;
            }
            memset(av, 0, PAGE_SIZE);
            
            av->desc = &arenas[arena_idx];
            av->large_request = 0;
            av->cnt = arenas[arena_idx].memory_blocks_sum;

            enum int_status old_status = DisableInt();

            for (size_t block_idx = 0; block_idx < av->cnt; block_idx++)
            {
                p = arena2block(av, block_idx);
                ASSERT(!elem_find(&av->desc->free_list, &p->free_elem));
                list_append(&av->desc->free_list, &p->free_elem);
            }

            SetIntStatus(old_status);
        }

        p = elem2entry(struct memory_block, free_elem, \
        list_pop(&arenas[arena_idx].free_list));
        memset(p, 0, arenas[arena_idx].block_size);
        av = block2arena(p);
        av->cnt--;
        sys_lock_unlock(&memory_pool->lock);
        return ((void*) p);
    }

}

void sys_free(void* p)
{
    if (p != NULL)
    {
        enum pool_flags PF;
        struct memory_pool* memory_pool;

        if (GetCurrentThreadPCB()->PDE_addr == NULL)
        {
            ASSERT((size_t) p >= KERNEL_HEAP_BASE);
            PF = KERNEL_POOL;
            memory_pool = &kernel_memory_pool;
        }
        else
        {
            PF = USER_POOL;
            memory_pool = &user_memory_pool;
        }

        sys_lock_lock(&memory_pool->lock);

        struct memory_block* victim = p;
        struct arena* av = block2arena(victim);
        ASSERT(av->large_request == 1 || av->large_request == 0);
        if (av->desc == NULL && av->large_request == 1)
        {
            pfree(PF, av, av->cnt);
        }
        else
        {
            list_append(&av->desc->free_list, &victim->free_elem);
            av->cnt++;
            if (av->cnt == av->desc->memory_blocks_sum)
            {
                for (size_t block_idx = 0; block_idx < av->desc->memory_blocks_sum; block_idx++)
                {
                    victim = arena2block(av, block_idx);
                    ASSERT(elem_find(&av->desc->free_list, &victim->free_elem));
                    list_remove(&victim->free_elem);
                }
                pfree(PF, av, 1);
            }
        }
        sys_lock_unlock(&memory_pool->lock);
    }
}
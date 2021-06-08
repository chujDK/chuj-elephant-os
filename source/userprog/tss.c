#include "tss.h"
#include "stdint.h"
#include "global.h"
#include "string.h"
#include "print.h"

/* 任务状态段tss结构 */
struct tss {
    uint32_t backlink;
    uint32_t* esp0;
    uint32_t ss0;
    uint32_t* esp1;
    uint32_t ss1;
    uint32_t* esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip) (void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t io_base;
}; 
static struct tss tss;

/* 更新tss中esp0字段的值为pthread的0级线 */
void UpdateTssEsp(struct task_struct* pthread) {
   tss.esp0 = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);
}

/* setup gdt desc */
static void SetUpGDTDesc(struct gdt_desc* desc_ptr, size_t* desc_base_addr, size_t limit, uint8_t attr_low, uint8_t attr_high)
{
    desc_ptr->limit_low_word = limit & 0x0000FFFF;
    desc_ptr->limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
    desc_ptr->base_low_word = (size_t) desc_base_addr & 0x0000FFFF;
    desc_ptr->base_mid_byte = (size_t) desc_base_addr & 0x00FF0000;
    desc_ptr->base_high_byte = (size_t) desc_base_addr >> 24;
    desc_ptr->attr_low_byte = (uint8_t)(attr_low);
}

/* create TSS, CODE, DATA desc in GDT, and reload GDT */
void TssInit() 
{
    sys_putstr("tss and ltr init..");
    size_t tss_size = sizeof(tss);
    memset(&tss, 0, tss_size);
    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;

    /* gdt_base: 0x600 + 0x10, tss on the 4th, which at 0x600 + 0x10 + 0x20 */
    SetUpGDTDesc((struct gdt_desc*) (0xC0000630),\
    (size_t *)&tss, tss_size - 1, TSS_ATTR_LOW, GDT_ATTR_HIGH);

    /* code DESC, dpl = 3 */
    SetUpGDTDesc((struct gdt_desc*) (0xC0000638),\
    (size_t *) 0, tss_size - 1, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    /* data, stack DESC, dpl = 3 */
    SetUpGDTDesc((struct gdt_desc*) (0xC0000640),\
    (size_t *) 0, tss_size - 1, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    uint64_t gdt_operand = \
    ((8 * 7 - 1) | ((uint64_t)((uint32_t)0xC0000610 << 16)));
    __asm__ volatile ("lgdt %0" : : "m" (gdt_operand));
    __asm__ volatile ("ltr %w0" : : "r" (SELECTOR_TSS));

    sys_putstr(" done\n");
}
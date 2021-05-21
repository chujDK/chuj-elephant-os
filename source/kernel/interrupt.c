#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"

#define PIC_MASTER_CTRL 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CTRL 0xa0
#define PIC_SLAVE_DATA 0xa1

#define IDT_DESC_SUM 0x21 /* sum of supported interrupts */

/* interrupt gate descriptor */
struct INT_gate_desc
{
    uint16_t function_offset_low_word;
    uint16_t selector;
    uint8_t dword_count; /* fixed value */
    uint8_t attribute;
    uint16_t function_offset_high_word;
};

static void MakeIdtDesc(struct INT_gate_desc* p_Gdesc, uint8_t attribute, intr_handler function);
static struct INT_gate_desc IDT[IDT_DESC_SUM]; /* Interrupt Descriptor Table */

extern intr_handler interrupt_entry_table[IDT_DESC_SUM]; /* Interrupt handle function */

static void MakeIdtDesc(struct INT_gate_desc* p_Gdesc, uint8_t attribute, intr_handler function)
{
    p_Gdesc->function_offset_low_word = ((uint32_t) function) & 0x0000FFFF;
    p_Gdesc->selector = SELECTOR_K_CODE;
    p_Gdesc->dword_count = 0;
    p_Gdesc->attribute = attribute;
    p_Gdesc->function_offset_high_word = ((((uint32_t) function)) & 0xFFFF0000) >> 16;
    return;
}

static void PicInit()
{
    sys_putstr("    pic_init..");
    /* Init Master */
    outb(PIC_MASTER_CTRL,0x11); /* ICW1 */
    outb(PIC_MASTER_DATA,0x20); /* ICW2 */
    outb(PIC_MASTER_DATA,0x04); /* ICW3 */
    outb(PIC_MASTER_DATA,0x01); /* ICW4 */

    /* Init Slave */
    outb(PIC_SLAVE_CTRL,0x11); /* ICW1 */
    outb(PIC_SLAVE_DATA,0x28); /* ICW2 */
    outb(PIC_SLAVE_DATA,0x02); /* ICW3 */
    outb(PIC_SLAVE_DATA,0x01); /* ICW4 */

    /* only respone to the clock(just for now) */
    outb(PIC_MASTER_DATA,0xFE);
    outb(PIC_SLAVE_DATA,0xFF);

    sys_putstr(" done\n");
}

static void IdtDescInit()
{
    sys_putstr("    idt_desc_init..");
    for (int i = 0; i < IDT_DESC_SUM; i++)
    {
        MakeIdtDesc(&IDT[i], IDT_DESC_ATTRIBUTE_DPL0, interrupt_entry_table[i]);
    }
    sys_putstr(" done\n");
}

void IdtInit()
{
    sys_putstr("setting up IDT..\n");
    IdtDescInit();
    PicInit(); /* init 8259A */

    /* load IDT */
    uint64_t idt_operand = ((sizeof(IDT) - 1) | ((uint64_t)((uint32_t) IDT << 16)));
    __asm__ volatile ("lidt %0": : "m" (idt_operand));
    sys_putstr(" done\n");
}

#include "interrupt.h"
#include "print.h"
#include "stdint.h"
#include "global.h"
#include "io.h"
#include "debug.h"

#define PIC_MASTER_CTRL 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CTRL 0xa0
#define PIC_SLAVE_DATA 0xa1

#define IDT_DESC_SUM 0x81 /* sum of supported interrupts */

#define EFLAGS_IF 0x00000200 /* IF = 1 */
#define GET_EFLAGS(EFLAGS_VAR) __asm__ volatile ("pushfl; popl %0" : "=g" (EFLAGS_VAR))

extern size_t syscall_handler();

/* interrupt gate descriptor */
struct INT_gate_desc
{
    uint16_t function_offset_low_word;
    uint16_t selector;
    uint8_t dword_count; /* fixed value */
    uint8_t attribute;
    uint16_t function_offset_high_word;
};

static void MakeIdtDesc(struct INT_gate_desc* p_Gdesc, uint8_t attribute, int_handler function);
static struct INT_gate_desc IDT[IDT_DESC_SUM]; /* Interrupt Descriptor Table */

extern int_handler interrupt_entry_table[IDT_DESC_SUM]; /* Interrupt handle function */
char* interrupt_name[IDT_DESC_SUM];
int_handler idt_table[IDT_DESC_SUM]; /* stores all the interrupt handle function */

void RegisterHandler(uint8_t int_vector_num, int_handler function)
{
    idt_table[int_vector_num] = function;
}

static void GeneralIntHandler(uint8_t int_vertor_number)
{
    DisableInt();
    sys_setcursor(0);
    if (int_vertor_number == 0x27 || int_vertor_number == 0x2F)
    {
        /* 0x27: spurious interrupt, needless to handle;
         * 0x2F: the last IRQ in the slave 8259A, still needless to handle */
        return;
    }
    sys_putstr("int vector: ");
    sys_putint(int_vertor_number);
    sys_putchar(' ');
    if (int_vertor_number <= 19)
    {
        sys_putstr(interrupt_name[int_vertor_number]);
    }
    sys_putchar('\n');
    while(1);
    return;
}

static void ExceptionInit()
{
    sys_putstr("    exception init..");
    for (int i = 0; i < IDT_DESC_SUM; i++)
    {
        idt_table[i] = GeneralIntHandler;
        interrupt_name[i] = "unkown";
    }
    interrupt_name[0] = "#DE Divide Error";
    interrupt_name[1] = "#DB Debug Exception";
    interrupt_name[2] = "NMI Interrupt";
    interrupt_name[3] = "#BP Breakpoint Exception";
    interrupt_name[4] = "#OF Overflow Exception";
    interrupt_name[5] = "#BR BOUND Range Exceeded Exception";
    interrupt_name[6] = "#UD Invalid Opcode Exception";
    interrupt_name[7] = "#NM Device Not Available Exception";
    interrupt_name[8] = "#DF Double Fault Exception";
    interrupt_name[9] = "Coprocessor Segment Overrun";
    interrupt_name[10] = "#TS Invalid TSS Exception";
    interrupt_name[11] = "#NP Segment Not Present";
    interrupt_name[12] = "#SS Stack Fault Exception";
    interrupt_name[13] = "#GP General Protection Exception";
    interrupt_name[14] = "#PF Page-Fault Exception";
    /* interrupt_name[15] is reserved,unused */
    interrupt_name[16] = "#MF x87 FPU Floating-Point Error";
    interrupt_name[17] = "#AC Alignment Check Exception";
    interrupt_name[18] = "#MC Machine-Check Exception";
    interrupt_name[19] = "#XF SIMD Floating-Point Exception";
    sys_putstr(" done\n");
    return;
}

static void MakeIdtDesc(struct INT_gate_desc* p_Gdesc, uint8_t attribute, int_handler function)
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

    /* open clock, keyboard, slave chip int */
    outb(PIC_MASTER_DATA,0xF8);
    /* open disk int */
    outb(PIC_SLAVE_DATA,0xBF);

    sys_putstr(" done\n");
}

static void IdtDescInit()
{
    sys_putstr("    idt_desc_init..");
    for (int i = 0; i < IDT_DESC_SUM; i++)
    {
        MakeIdtDesc(&IDT[i], IDT_DESC_ATTRIBUTE_DPL0, interrupt_entry_table[i]);
    }

    ASSERT(IDT_DESC_SUM > 0x80);
    /* syscall support */
    MakeIdtDesc(&IDT[0x80], IDT_DESC_ATTRIBUTE_DPL3, syscall_handler);
    sys_putstr(" done\n");
}

enum int_status EnableInt()
{
    enum int_status old_status;
    if (INT_ON == GetIntStatus())
    {
        old_status = INT_ON;
        return old_status;
    }
    else
    {
        old_status = INT_OFF;
        __asm__ volatile ("sti" : : : "memory"); /* set IF = 1 */
        return old_status;
    }
}

enum int_status DisableInt()
{
    enum int_status old_status;
    if (INT_ON == GetIntStatus())
    {
        old_status = INT_ON;
        __asm__ volatile ("cli" : : : "memory"); /* set IF = 0 */
        return old_status;
    }
    else
    {
        old_status = INT_OFF;
        return old_status;
    }
}

enum int_status GetIntStatus()
{
    uint32_t eflags = 0;
    GET_EFLAGS(eflags);
    return (eflags & EFLAGS_IF) ? INT_ON : INT_OFF;
}

enum int_status SetIntStatus(enum int_status status)
{
    return status & INT_ON ? EnableInt() : DisableInt();
}

void IdtInit()
{
    sys_putstr("setting up IDT..\n");
    IdtDescInit();
    ExceptionInit();
    PicInit(); /* init 8259A */

    /* load IDT */
    uint64_t idt_operand = ((sizeof(IDT) - 1) | ((uint64_t)((uint32_t) IDT << 16)));
    __asm__ volatile ("lidt %0": : "m" (idt_operand));
    sys_putstr(" done\n");
}

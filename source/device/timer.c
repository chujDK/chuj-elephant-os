#include "timer.h"
#include "io.h"
#include "print.h"

#define IRQ0_FREQUENCY      80
#define PULSE_FREQUENCY     1193180
#define COUNTER0_VALUE      PULSE_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT       0x40
#define COUNTER0_NUM        0
#define COUNTER0_MODE       2 /* Rate Generator Mode */
#define READ_WRITE_LATCH    3
#define PIT_CONTROL_PORT    0x43 /* control word register port */

static void FrequencySet(uint8_t counter_port, \
                         uint8_t counter_num, \
                         uint8_t rwl,
                         uint8_t counter_mode, \
                         uint8_t counter_value)
{
    /* wirte control word */
    outb(counter_port, \
         (uint8_t) ((counter_num << 6) | (rwl << 4) | (counter_mode << 1)));
    /* low 8 bits of counter_value */
    outb(counter_port, (uint8_t) counter_value);
    /* high 8 bits of counter_value */
    outb(counter_port, (uint8_t) counter_value >> 8);
}

void TimerInit()
{
    sys_putstr("timer init..");
    FrequencySet(COUNTER0_PORT, \
                 COUNTER0_NUM, \
                 READ_WRITE_LATCH, \
                 COUNTER0_MODE, \
                 COUNTER0_VALUE);
    sys_putstr(" done\n");
}

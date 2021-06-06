#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "stdint.h"
#include "ioqueue.h"

#define KEYBOARD_BUF_PORT 0x60

/* 用转义字符定义部分控制字符 */
#define esc		'\033'	 // 八进制表示字符,也可以用十六进制'\x1b'
#define backspace	'\b'
#define tab		'\t'
#define enter		'\r'
#define delete		'\177'	 // 八进制表示字符,十六进制为'\x7f'

/* 以上不可见字符一律定义为0 */
#define char_invisible	0
#define ctrl_l_char	char_invisible
#define ctrl_r_char	char_invisible
#define shift_l_char	char_invisible
#define shift_r_char	char_invisible
#define alt_l_char	char_invisible
#define alt_r_char	char_invisible
#define caps_lock_char	char_invisible

/* 定义控制字符的通码和断码 */
#define shift_l_make	0x2a
#define shift_r_make 	0x36 
#define alt_l_make   	0x38
#define alt_r_make   	0xe038
#define alt_r_break   	0xe0b8
#define ctrl_l_make  	0x1d
#define ctrl_r_make  	0xe01d
#define ctrl_r_break 	0xe09d
#define caps_lock_make 	0x3a

struct ioqueue keyboard_IO_buf;

/* 定义以下变量记录相应键是否按下的状态,
 * ext_scancode用于记录makecode是否以0xe0开头 */
static uint8_t ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

/* 以通码make_code为索引的二维数组 */
static char keymap[][2] = {
/* 扫描码   未与shift组合  与shift组合*/
/* ---------------------------------- */
/* 0x00 */	{0,	0},		
/* 0x01 */	{esc,	esc},		
/* 0x02 */	{'1',	'!'},		
/* 0x03 */	{'2',	'@'},		
/* 0x04 */	{'3',	'#'},		
/* 0x05 */	{'4',	'$'},		
/* 0x06 */	{'5',	'%'},		
/* 0x07 */	{'6',	'^'},		
/* 0x08 */	{'7',	'&'},		
/* 0x09 */	{'8',	'*'},		
/* 0x0A */	{'9',	'('},		
/* 0x0B */	{'0',	')'},		
/* 0x0C */	{'-',	'_'},		
/* 0x0D */	{'=',	'+'},		
/* 0x0E */	{backspace, backspace},	
/* 0x0F */	{tab,	tab},		
/* 0x10 */	{'q',	'Q'},		
/* 0x11 */	{'w',	'W'},		
/* 0x12 */	{'e',	'E'},		
/* 0x13 */	{'r',	'R'},		
/* 0x14 */	{'t',	'T'},		
/* 0x15 */	{'y',	'Y'},		
/* 0x16 */	{'u',	'U'},		
/* 0x17 */	{'i',	'I'},		
/* 0x18 */	{'o',	'O'},		
/* 0x19 */	{'p',	'P'},		
/* 0x1A */	{'[',	'{'},		
/* 0x1B */	{']',	'}'},		
/* 0x1C */	{enter,  enter},
/* 0x1D */	{ctrl_l_char, ctrl_l_char},
/* 0x1E */	{'a',	'A'},		
/* 0x1F */	{'s',	'S'},		
/* 0x20 */	{'d',	'D'},		
/* 0x21 */	{'f',	'F'},		
/* 0x22 */	{'g',	'G'},		
/* 0x23 */	{'h',	'H'},		
/* 0x24 */	{'j',	'J'},		
/* 0x25 */	{'k',	'K'},		
/* 0x26 */	{'l',	'L'},		
/* 0x27 */	{';',	':'},		
/* 0x28 */	{'\'',	'"'},		
/* 0x29 */	{'`',	'~'},		
/* 0x2A */	{shift_l_char, shift_l_char},	
/* 0x2B */	{'\\',	'|'},		
/* 0x2C */	{'z',	'Z'},		
/* 0x2D */	{'x',	'X'},		
/* 0x2E */	{'c',	'C'},		
/* 0x2F */	{'v',	'V'},		
/* 0x30 */	{'b',	'B'},		
/* 0x31 */	{'n',	'N'},		
/* 0x32 */	{'m',	'M'},		
/* 0x33 */	{',',	'<'},		
/* 0x34 */	{'.',	'>'},		
/* 0x35 */	{'/',	'?'},
/* 0x36	*/	{shift_r_char, shift_r_char},	
/* 0x37 */	{'*',	'*'},    	
/* 0x38 */	{alt_l_char, alt_l_char},
/* 0x39 */	{' ',	' '},		
/* 0x3A */	{caps_lock_char, caps_lock_char}
/*其它按键暂不处理*/
};

static void IntKeyboardHandler()
{
    uint8_t ctrl_down_last = ctrl_status;
    uint8_t shift_down_last = shift_status;
    uint8_t caps_lock_down_last = caps_lock_status;

    uint8_t is_break_code;
    uint16_t scan_code = inb(KEYBOARD_BUF_PORT);

    if (scan_code == 0xE0)
    {
        ext_scancode = 1;
        return;
    }

    if (ext_scancode)
    {
        scan_code = ((0xE000) | scan_code);
        ext_scancode = 0;
    }

    is_break_code = ((scan_code & 0x0080) != 0);

    if (is_break_code)
    {
        /* 
         * we only care if ctrl, shift, alt is break 
         * anything else can be ignored 
         */
        uint16_t make_code = (scan_code &= 0xFF7F);
        if (make_code == ctrl_l_make || make_code == ctrl_r_make)
        {
            ctrl_status = 0;
        }
        if (make_code == shift_l_make || make_code == shift_r_make)
        {
            shift_status = 0;
        }
        if (make_code == alt_l_make || make_code == alt_r_make)
        {
            alt_status = 0;
        }
    }
    else if ((scan_code > 0x00 && scan_code < 0x3B) || \
             (scan_code == alt_r_make) || \
             (scan_code == ctrl_r_make))
        {
            uint8_t shift_mode = 0;
            if ((scan_code <  0x0e) || (scan_code == 0x29) || \
	            (scan_code == 0x1a) || (scan_code == 0x1b) || \
	            (scan_code == 0x2b) || (scan_code == 0x27) || \
	            (scan_code == 0x28) || (scan_code == 0x33) || \
	            (scan_code == 0x34) || (scan_code == 0x35)) 
            {  	    /****** 代表两个字母的键 ********
                        0x0e 数字'0'~'9',字符'-',字符'='
                        0x29 字符'`'
                        0x1a 字符'['
                        0x1b 字符']'
                        0x2b 字符'\\'
                        0x27 字符';'
                        0x28 字符'\''
                        0x33 字符','
                        0x34 字符'.'
                        0x35 字符'/' 
                    *******************************/
                if (shift_down_last)
                {
                    shift_mode = 1;
                }
            }
            else /* this case is controled by caps_lock */
            {
                if (shift_down_last ^ caps_lock_down_last)
                {
                    shift_mode = 1;
                }
                else
                {
                    shift_mode = 0;
                }
            }

            uint8_t current_idx = (scan_code &= 0x00FF);
            char current_char = keymap[current_idx][shift_mode];

            if (current_char)
            {
                sys_putchar(current_char); /* tmp */
                ioqueue_putchar(&keyboard_IO_buf, current_char);
            }

            if (scan_code == ctrl_l_make || scan_code == ctrl_r_make) 
            {
	            ctrl_status = 1;
            } 
            else if (scan_code == shift_l_make || scan_code == shift_r_make) 
            {
	            shift_status = 1;
            } 
            else if (scan_code == alt_l_make || scan_code == alt_r_make) 
            {
	            alt_status = 1;
            } 
            else if (scan_code == caps_lock_make) 
            {
                /* 不管之前是否有按下caps_lock键,当再次按下时则状态取反,
                * 即:已经开启时,再按下同样的键是关闭。关闭时按下表示开启。*/
	            caps_lock_status = !caps_lock_status;
            }
        }
        else
        {
            sys_putstr("unknown key!\n");
        }
    return;
}

void KeyboardInit()
{
    sys_putstr("keyboard_init..");
    ioqueueInit(&keyboard_IO_buf);
    RegisterHandler(0x21, IntKeyboardHandler);
    sys_putstr(" done\n");
}

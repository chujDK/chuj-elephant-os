; a simply test of INTeRrupt
[bits 32]
%define HAS_ERROR_CODE nop  
%define NONE_ERROR_CODE push 0
; when this is no error code pushed, push a 0 manually
; then we can process easier
extern idt_table

section .data
global interrupt_entry_table
interrupt_entry_table:

%macro VECTOR 2                                 ; VECTOR INTnumber HAS(NONE)_ERROR_CODE
section .text
INTR%1Entry:
    %2                                          ; make all kinds of INTeRrupt
                                                ; has the same stack struct
                                                ; means a int with error code, push noting
                                                ; a int without error code, push a "error code"
    push ds
    push es
    push fs
    push gs
    pushad                                      ; save the context
    
    ; send EOI(End Of Interrupt) to the 8259A
    mov al,0x20                                 ; EOI
    out 0xa0,al                                 ; send to slave
    out 0x20,al                                 ; send to master

    push %1                                     ; push it anyway
    call [idt_table + %1 * 4]                   ; the the C function
    jmp IntExit

section .data
    dd INTR%1Entry
%endmacro

section .text
global IntExit                                  
; ---------- this function mainly restored the context ----------
IntExit:
    add esp,4                                   ; pass the int number
    popad
    pop gs
    pop fs
    pop es
    pop ds                                      ; restore the context
    add esp,4                                   ; pass the error code
    iretd

VECTOR 0x00,NONE_ERROR_CODE
VECTOR 0x01,NONE_ERROR_CODE
VECTOR 0x02,NONE_ERROR_CODE
VECTOR 0x03,NONE_ERROR_CODE
VECTOR 0x04,NONE_ERROR_CODE
VECTOR 0x05,NONE_ERROR_CODE
VECTOR 0x06,NONE_ERROR_CODE
VECTOR 0x07,NONE_ERROR_CODE
VECTOR 0x08,NONE_ERROR_CODE
VECTOR 0x09,NONE_ERROR_CODE
VECTOR 0x0A,NONE_ERROR_CODE
VECTOR 0x0B,NONE_ERROR_CODE
VECTOR 0x0C,NONE_ERROR_CODE
VECTOR 0x0D,NONE_ERROR_CODE
VECTOR 0x0E,NONE_ERROR_CODE
VECTOR 0x0F,NONE_ERROR_CODE
VECTOR 0x10,NONE_ERROR_CODE
VECTOR 0x11,NONE_ERROR_CODE
VECTOR 0x12,NONE_ERROR_CODE
VECTOR 0x13,NONE_ERROR_CODE
VECTOR 0x14,NONE_ERROR_CODE
VECTOR 0x15,NONE_ERROR_CODE
VECTOR 0x16,NONE_ERROR_CODE
VECTOR 0x17,NONE_ERROR_CODE
VECTOR 0x18,NONE_ERROR_CODE
VECTOR 0x19,NONE_ERROR_CODE
VECTOR 0x1A,NONE_ERROR_CODE
VECTOR 0x1B,NONE_ERROR_CODE
VECTOR 0x1C,NONE_ERROR_CODE
VECTOR 0x1D,NONE_ERROR_CODE
VECTOR 0x1E,HAS_ERROR_CODE
VECTOR 0x1F,NONE_ERROR_CODE
VECTOR 0x20,NONE_ERROR_CODE



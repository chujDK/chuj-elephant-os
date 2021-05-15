%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp LoaderStart

; set up GOT and descriptor
GDT_BASE: dd 0x00000000             
          dd 0x00000000             

CODE_DESC: dd 0x0000FFFF                ; low 32 bits
           dd DESC_CODE_HIGH4           ; high 32 bits

DATA_STACK_DESC: dd 0x0000FFFF          ; used by stack and data seg
            dd DESC_DATA_HIGH4

; text-mode display
; limit = (0xBFFFF - 0xB8000) / 4K = 0x7
VIDEO_DESC: dd 0x80000007 
            dd DESC_VIDEO_HIGH4

GDT_SIZE  equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

times 60 dq 0                           ; reserve 60 GDTs

SELECTOR_CODE equ ((CODE_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0
SELECTOR_DATA equ ((DATA_STACK_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0
SELECTOR_VIDEO equ ((VIDEO_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0

; pointer point to GDT
gdt_ptr: dw GDT_LIMIT    ; low 16 bits of GDT reg
         dd GDT_BASE     ; high 32 bits of GDT reg
; end of GDT setup


loader_msg db '2 loader in real mode.'

LoaderStart:
mov sp,LOADER_BASE_ADDR
mov bp,loader_msg
mov cx,22               ; len
mov ax,0x1301           ; 0x13: int mode, 0x01: display mode
mov bx,0x001F           ; page num 0, color: 0x1F
mov dx,0x1800           ; place to display
int 0x10                ; call int

; ---------- ready to enter Proctection mode ----------
; 1 open A20 address line
; 2 load GDT reg
; 3 set pe of cr0 to 1

; open A20
in al,0x92              ;
or al,0000_0010B        ; save existed status
out 0x92,al
; load GDT reg
lgdt [gdt_ptr]
; set cr0, let's roll!
mov eax,cr0
or eax,0x00000001       ; save existed status
mov cr0,eax             ; enter Protection mode

jmp dword SELECTOR_CODE:ProctectionModeStart    ; reflesh assembly line

[bits 32]
ProctectionModeStart:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax

    mov byte [gs:160],'P'

    jmp $

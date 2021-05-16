%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp LoaderStart                         ; 3 bytes
db 0
dd 0,0,0                                ; addr align
; offset 0x10

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

TOTAL_MEM_BYTES dd 0                    ; memory of the machine
                                        ; addr: LOADER_BASE_ADDR + 0x10 + 0x200 = 0x800

SELECTOR_CODE equ ((CODE_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0
SELECTOR_DATA equ ((DATA_STACK_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0
SELECTOR_VIDEO equ ((VIDEO_DESC - GDT_BASE) / 8) << 3 + TI_GDT + RPL0

; pointer point to GDT
gdt_ptr: dw GDT_LIMIT    ; low 16 bits of GDT reg
         dd GDT_BASE     ; high 32 bits of GDT reg
; end of GDT setup

LoaderStart:
; ---------- first, get the total memory of the machine ----------
; ---------- we must do it before enter the PE mode as we need the BIOS int ----------
; use bios int 0x15 sub 0xE801
.loader_E801FailedRetry:
mov ax,0xE801
int 0x15
jc .loader_E801FailedRetry
; calculate low 15MB memory
mov cx,0x400
mul cx
shl edx,16
and eax,0x0000FFFF
or edx,eax
add edx,0x100000                        ; add 1MB, this is caused by the memory hole
mov esi,edx

xor eax,eax
mov ax,bx
mov ecx,0x10000                         ; 64 * 1024
mul ecx
add esi,eax                             ; esi store the
mov [TOTAL_MEM_BYTES],esi               ; now TOTAL_MEM_BYTES stores the total memory

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

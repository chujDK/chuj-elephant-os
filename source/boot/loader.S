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
    .LoaderStart_E801FailedRetry:
        mov ax,0xE801
        int 0x15
    jc .LoaderStart_E801FailedRetry
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
    in al,0x92                             
    or al,0000_0010B                        ; save existed status
    out 0x92,al
    ; load GDT reg
    lgdt [gdt_ptr]
    ; set cr0, let's roll!
    mov eax,cr0
    or eax,0x00000001                       ; save existed status
    mov cr0,eax                             ; enter Protection mode

    jmp dword SELECTOR_CODE:ProctectionModeStart    ; reflesh assembly line
; ---------- end of function LoaderStart ----------

; ---------- now we are in 32-bits PE mode ----------
[bits 32]
ProctectionModeStart:
; set selectors
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:160],'P'

; first thing we do is start the page mode
    ; 1 setup PDE and related PTE
    call SetupPage
    ; 2 modify the GDT to make it work in paging mode
    sgdt [gdt_ptr]
    mov ebx,[gdt_ptr + 2]
    or dword [ebx + 0x18 + 4],0xC0000000    ; modify the VIDEO_DESC

    add dword [gdt_ptr + 2],0xC0000000      ; pre modify the GDTR value 
    add esp,0xC0000000                      ; also modify the stack 

    mov eax,PAGE_DIR_TABLE_POS
    mov cr3,eax

    mov eax,cr0
    or eax,0x80000000                       ; save existed status
    mov cr0,eax                             ; enable paging mode
    
    lgdt [gdt_ptr]                          ; change GDTR

    mov byte [gs:162],'V'
    jmp $

SetupPage:
; ---------- this function setup the Page Directory Entry and Page Table Entry ----------
    ; clear PTE
    mov ecx,0x1000                          ; 4K PDE
    mov esi,0                               ; use this reg the clear
    .SetupPage_ClearPDE:
        mov byte [PAGE_DIR_TABLE_POS + esi],0
        inc esi
    loop .SetupPage_ClearPDE

    ; setup PDE
    .SetupPage_CreatePDE:
        mov eax,PAGE_DIR_TABLE_POS
        add eax,0x1000                       ; addr of the first PTE 
        mov ebx,eax                          ; ebx is the base addr of PTEs

        ; make the PDE[0] and PDE[0xC00] point to the first PTE
        or eax,PG_US_U | PG_RW_RW | PG_P     ; set user page status
        mov [PAGE_DIR_TABLE_POS + 0x0],eax   ; the first PTE's place
        mov [PAGE_DIR_TABLE_POS + 0xC00],eax ; the first PTE used by kernel, point ot the fiset PTE
        ; 0xC0000000 ~ 0xFFFFFFFF belongs to kernel
        sub eax,0x1000
        mov [PAGE_DIR_TABLE_POS + 4092],eax  ; make the last Entry point to PDE itself

        ; creat PTE for kernel
        mov ecx,256                          ; 1M / 4K = 256
        mov esi,0
        mov edx,PG_US_U | PG_RW_RW | PG_P    ; User, RW, P
        .SetupPage_CreatePTE:
            mov [ebx + esi * 4],edx
            add edx,0x1000
            inc esi
        loop .SetupPage_CreatePTE

        mov eax,PAGE_DIR_TABLE_POS
        add eax,0x2000                      ; second PTE
        or eax,PG_US_U | PG_RW_RW | PG_P
        mov ebx,PAGE_DIR_TABLE_POS
        mov ecx,254                         ; 1022 - 769 + 1
        mov eax,769                         ; start from 769,the second PTE of kernel
        .SetupPage_CreateKernelPDE:
            mov [ebx + esi * 4],eax
            inc esi
            add eax,0x1000
        loop .SetupPage_CreateKernelPDE
        ret
; ---------- end of function SetupPage ----------




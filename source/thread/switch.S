[bits 32]
section .text
global switch_to
switch_to:
; callee only backup esi, edi, ebx, ebp for caller
    push esi
    push edi
    push ebx
    push ebp

    mov eax,[esp + 20]      ; get current_thread
    mov [eax],esp           ; store current stack top to self_kernel_stack

    mov eax,[esp + 24]      ; get next_thread
    mov esp,[esp]           ; get next_thread's self_kernel_stack back
    pop ebp
    pop ebx
    pop edi
    pop esi                 
    ret

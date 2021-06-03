TI_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text

; -------------------- sys_putstr --------------------
; write a string (end by '\0')
; ----------------------------------------------------
global sys_putstr
sys_putstr:
    push ecx
    push ebx
    mov ebx,[esp + 12]
    xor ecx,ecx
    xor eax,eax
    .sys_putstr_PutNext:
        mov cl,[ebx]
        test cl,cl
        jz .sys_putstr_EndOfStr
        push ecx
        call sys_putchar
        add esp,4
        inc ebx
        inc eax
        jmp .sys_putstr_PutNext
    .sys_putstr_EndOfStr:
    pop ebx
    pop ecx
    ret
; -------------------- end of function sys_putstr --------------------

; -------------------- sys_putchar --------------------
; write one char in stack to the cursor
; -----------------------------------------------------
global sys_putchar
sys_putchar:
    pushad                                          ; backup all regs (8 * 4 = 32bytes)
    mov ax,SELECTOR_VIDEO
    mov gs,ax                                       ; make sure gs stores the right selector

    ; get the current cursor addr (high 8 bits)
    mov dx,0x3D4                                    ; Address Reg (base)
    mov al,0x0E                                     ; Cursor Location High Reg (idx)
    out dx,al                                       
    mov dx,0x3D5                                    ; Data Reg (base)
    in al,dx                                        ; get the high 8 bits of the cursor addr
    mov ah,al

    ; get the current cursor addr (low 8 bits)
    mov dx,0x3D4                                    ; Address Reg (base)
    mov al,0x0F                                     ; Cursor Location Low Reg (idx)
    out dx,al                                       
    mov dx,0x3D5                                    ; Data Reg (base)
    in al,dx                                        ; get the low 8 bits of the cursor addr
    
    ; save the cursor addr to bx
    mov bx,ax

    ; get the char wating to be put
    mov ecx,[esp + 36]                              ; 32(backup regs) + 4(return addr) = 36
    cmp cl,0x0d                                     ; CR(Carriage Return): 0x0d
    jz .sys_putchar_CarriageReturn
    cmp cl,0x0a                                     ; LF(Line Feed): 0x0a
    jz .sys_putchar_LineFeed
    cmp cl,0x08                                     ; BF(BackSpace): 0x08
    jz .sys_putchar_BackSpace
    jmp .sys_putchar_AnyOther                       ; Any other char

    .sys_putchar_BackSpace:
        dec bx                                      ; cursor back one step
        shl bx,1                                    ; bx<<1 <=> bx * 2
        mov byte [gs:bx],0x20                       ; fill the delete char with ' '
        inc bx
        mov byte [gs:bx],0x07                       ; 00000111b, (default black back,withe front)
        shr bx,1                                    ; bx>>1 ,=> bx // 2 
        jmp .sys_putchar_SetCursor

    .sys_putchar_AnyOther:
        shl bx,1                                    ; bx<<1
        mov byte byte[gs:bx],cl                     ; put the char
        inc bx
        mov byte byte[gs:bx],0x07                   ; set the statu
        inc bx                                      ; point to the next char
        shr bx,1                                    ; bx>>1
        cmp bx,2000                                 ; bx == 2000, don't jmp, bx < 2000, jmp
        jl .sys_putchar_SetCursor                   ; if the cursor overflow the maximum of the 
                                                    ; video memory, do a Line Feed, if not, set 
                                                    ; the new cursor.
    .sys_putchar_LineFeed:
    .sys_putchar_CarriageReturn:
        xor dx,dx                                   ; high 16 bits of the number to be div
        mov bx,bx                                   ; low 16 bits of the number to be div
        mov si,80                                   ; diver
        div si                                      
        sub bx,dx                                   ; bx = bx - bx % 80 => make the cursor point to the front of the line
                                                    ; CR done 
        add bx,80                                   ; dx = dx + 80 => point to the next line
                                                    ; LF done
        cmp bx,2000
        jl .sys_putchar_SetCursor

    .sys_putchar_RollOneLine:                       ; move line 1~24 to the line 0~23 and clear the last line
        ; move line 1~24 to the line 0~23
        mov ecx,960                                 ; ((2000 - 80) * 2)(byte) / 4 =960(dword) 
        mov esi,0xC00B80A0                          ; front of line 1
        mov edi,0xC00B8000                          ; front of line 0
        cld                                         ; increase copy
        rep movsd

        ; clear the last line
        mov ecx,80                                  ; 80 words (only one word at a time)
        mov ebx,3840                                ; (2000 - 80) * 2 = 3840
        .sys_putchar_RollOneLine_CLL:
            mov word [gs:ebx],0x0720                ; blank
            add ebx,2
            loop .sys_putchar_RollOneLine_CLL
        mov bx,1920                                 ; make cursor point to the last line

    .sys_putchar_SetCursor:
        ; set the current cursor addr (high 8 bits)
        mov dx,0x3D4                                ; Address Reg (base)
        mov al,0x0E                                 ; Cursor Location High Reg (idx)
        out dx,al                                      
        mov dx,0x3D5                                ; Data Reg (base)
        mov al,bh
        out dx,al                                   ; set the high 8 bits of the cursor addr

        ; set the current cursor addr (low 8 bits)
        mov dx,0x3D4                                ; Address Reg (base)
        mov al,0x0F                                 ; Cursor Location low Reg (idx)
        out dx,al                                       
        mov dx,0x3D5                                ; Data Reg (base)
        mov al,bl
        out dx,al                                   ; set the low 8 bits of the cursor addr

    popad                                           ; reset the regs
    ret
; -------------------- end of function sys_putchar --------------------

global sys_setcursor
sys_setcursor:
    push ebx
    mov bx,si
    mov dx,0x3D4                                ; Address Reg (base)
    mov al,0x0E                                 ; Cursor Location High Reg (idx)
    out dx,al                                      
    mov dx,0x3D5                                ; Data Reg (base)
    mov al,bh
    out dx,al                                   ; set the high 8 bits of the cursor addr

    ; set the current cursor addr (low 8 bits)
    mov dx,0x3D4                                ; Address Reg (base)
    mov al,0x0F                                 ; Cursor Location low Reg (idx)
    out dx,al                                       
    mov dx,0x3D5                                ; Data Reg (base)
    mov al,bl
    out dx,al                                   ; set the low 8 bits of the cursor addr
    pop ebx
    ret
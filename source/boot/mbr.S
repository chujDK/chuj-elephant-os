%include "boot.inc"

SECTION MBR vstart=0x7C00
    ; init
    mov ax,cx
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7C00
    mov ax,0xb800
    mov gs,ax

    ; clear screen
    mov ax,0x0600                   ; AH:clear. AL:colum to clear, 0 as all
    mov bx,0x0700                   ; BH:colum nature
    mov cx,0                        ; (CL,CH) (x,y) of left-up corner
    mov dx,0x184F                   ; (DL,DH) (x,y) of right-down corner (80-1,25-1)
    int 0x10                        ; use bios interrupt

    ; display "1 MBR"
    mov byte [gs:0x00],'M'

    ; ready to call kernel loader
    mov esi,LOADER_START_SECTOR     ; LBA addr of start sector
    mov di,LOADER_BASE_ADDR         ; writing addr
    mov dx,4                        ; sectors waiting to read
    call ReadDiskSector_16
    jmp LOADER_BASE_ADDR           ; call loader
;---------- end of mbr ----------

; function ReadDiskSector_16(LBA_addr, writing_addr, n), read n sectors from hard-disk in 16 bit mode
; esi: LBA addr of start sector
; di: writing addr
; dx: n
ReadDiskSector_16:
        ; read sectors
        mov bx,dx                       ; bx keeps the n
        mov al,bl                       ; n sectors
        mov dx,0x1F2                    ; set reg Sector count 
        out dx,al                       ; read n sectors

        ; set LBA addr
        mov eax,esi
        mov dx,0x1F3                    ; set reg LBA low
        out dx,al                       ; write low 8 bits

        mov cl,8
        shr eax,cl
        mov dx,0x1F4                    ; set reg LBA mid
        out dx,al                       ; write LBA mid

        shr eax,cl
        mov dx,0x1F5                    ; set reg LBA high
        out dx,al                       ; write LBA high

        shr eax,cl
        and al,0xF                      ; only 4 bits
        or al,0xE0                      ; 1110b: LBA mode, disk: master
        mov dx,0x1F6                    ; set reg device
        out dx,al                       ; set mode and LBA addr

        ; ready to read
        mov dx,0x1F7                    ; set reg command
        mov al,0x20                     ; mode: read
        out dx,al                       ; do read

        ; check disk status
    .ReadDiskSector_16_DiskNotReady:
        in al,dx                        ; get disk status
        and al,0x88                     ; result 0x8 => disk is read 
                                        ; result 0x80 => disk is busy
        cmp al,0x08
        jnz .ReadDiskSector_16_DiskNotReady

        ; read data
        mov ax,bx                       ; get n
        mov dx,256                      ; read by word, so dx = 512 / 2
        mul dx                          ; assum this mul won't overflow
        mov cx,ax                       ; sum of words need to read
        mov dx,0x1F0                    ; set reg data
    .ReadDiskSector_16_ReadingLoop:
        in ax,dx                        ; read a word
        mov [di],ax                     ; write a word
        add di,2
        loop .ReadDiskSector_16_ReadingLoop
        ret
; end of function ReadDiskSector_16

    times 510 - ($ - $$) db 0
    db 0x55,0xaa                        ; magic number

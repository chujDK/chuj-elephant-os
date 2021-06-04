;---------- loader and kernel ----------
LOADER_BASE_ADDR equ 0x600                  ; 0x500 ~ 0x7BFF
LOADER_START_SECTOR equ 0x2

;---------- gdt related ----------
; G, D, L, AVL sign
DESC_G_4K equ 1_00000000000000000000000b    ; set grid 4K
DESC_D_32 equ  1_0000000000000000000000b    ; set 32 bit text mode
DESC_L    equ   0_000000000000000000000b    ; turn off 64 bit text mode
DESC_AVL  equ    0_00000000000000000000b    ; unused by CPU

; segment limit high 4 bits
DESC_LIMIT_CODEH equ 1111_0000000000000000b ; LIMIT 0xF(FFFF)
DESC_LIMIT_DATAH equ DESC_LIMIT_CODEH       ; LIMIT 0xF(FFFF)
DESC_LIMIT_VIDEOH equ 0000_0000000000000000b 

; Present sign
DESC_P_IN equ 1_000000000000000b            ; this segment is in RAM

; Descriptor Privilege Level (DPL sign)
DESC_DPL_RING_0 equ 00_0000000000000b       ; set RING 0 
DESC_DPL_RING_1 equ 01_0000000000000b       ; set RING 1 
DESC_DPL_RING_2 equ 10_0000000000000b       ; set RING 2 
DESC_DPL_RING_3 equ 11_0000000000000b       ; set RING 3 

; CPU segment status (S sign)
DESC_S_CODE equ 1_000000000000b             ; code segment
DESC_S_DATA equ DESC_S_CODE                 ; data segment
DESC_S_SYS  equ 0_000000000000b             ; sys segment (to cpu)

; OS segment status (type sign)
DESC_TYPE_CODE equ 1000_00000000b           ; code segment (r-x)
DESC_TYPE_DATA equ 0010_00000000b           ; data segment (rw-) 

; normalized Descriptor
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_L + \
DESC_D_32 + DESC_AVL + DESC_P_IN + DESC_LIMIT_CODEH + \
DESC_DPL_RING_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_L + \
DESC_D_32 + DESC_AVL + DESC_P_IN + DESC_LIMIT_DATAH + \
DESC_DPL_RING_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_L + \
DESC_D_32 + DESC_AVL + DESC_P_IN + DESC_LIMIT_VIDEOH + \
DESC_DPL_RING_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

;---------- selector status ----------
; Request Privilege Level
RPL0 equ 00b                                ; Ring 0
RPL1 equ 01b                                ; Ring 1
RPL2 equ 10b                                ; Ring 2
RPL3 equ 11b                                ; Ring 3

; Table Indicator
TI_GDT equ 000b                                 ; set GDT selector 
TI_LDT equ 100b                                 ; set LDT selector

; ---------- page related ----------
PAGE_DIR_TABLE_POS equ 0x10000              ; PDT start at 1M

PG_P     equ 1                              ; Present sign
PG_RW_RW equ 10                             ; page type:rw-
PG_RW_R  equ 00                             ; page type:r--
PG_US_U  equ 100                            ; User level
PG_US_S  equ 000                            ; Supervisor level

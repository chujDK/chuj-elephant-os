#!/bin/bash
./gcc-c.sh main.c
ld main.o -Ttext 0xc0001500 -e _start -o kernel.bin -m elf_i386
dd if=./kernel.bin of=../../disk/hd60M.img bs=512 count=200 seek=9 conv=notrunc

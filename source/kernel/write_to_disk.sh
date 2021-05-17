#!/bin/bash
./gcc-c.sh main.c
dd if=./kernel.bin of=../../disk/hd60M.img bs=512 count=200 seek=9 conv=notrunc

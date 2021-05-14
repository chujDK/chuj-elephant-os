#!/bin/bash
# complie mbr
nasm -I include/ -o mbr.bin mbr.S
# write mbr
dd if=./mbr.bin of=../../disk/hd60M.img bs=512 count=1 seek=0 conv=notrunc
# complie loader
nasm -I include/ -o loader.bin loader.S
# write loader
dd if=./loader.bin of=../../disk/hd60M.img bs=512 count=1 seek=2 conv=notrunc

DIR_BOOT = ./source/boot
DIR_KERNEL = ./source/kernel
DIR_LIB = ./source/lib

## boot related
SRC_BOOT = ${DIR_BOOT}/*.S
DIR_BOOT_INC = ${DIR_BOOT}/include

BIN = ./bin
DIR_DISK = ./disk

${DIR_DISK}/bochsrc.disk : ${BIN}/kernel.bin ${BIN}/mbr.bin ${BIN}/loader.bin
	@echo "writing to disk .."
	bximage -mode=create -hd=60M -sectsize=512 -q ${DIR_DISK}/hd60M.img
	dd if=${BIN}/mbr.bin of=${DIR_DISK}/hd60M.img bs=512 count=1 seek=0 conv=notrunc
	dd if=${BIN}/loader.bin of=${DIR_DISK}/hd60M.img bs=512 count=4 seek=2 conv=notrunc
	dd if=${BIN}/kernel.bin of=${DIR_DISK}/hd60M.img bs=512 count=200 seek=9 conv=notrunc


${BIN}/mbr.bin : ${DIR_BOOT}/mbr.S
	@echo "making mbr.bin .."
	$(shell mkdir -p ./bin)
	nasm -I ${DIR_BOOT_INC}/ -o ${BIN}/mbr.bin ${DIR_BOOT}/mbr.S 

${BIN}/loader.bin : ${DIR_BOOT}/loader.S
	@echo "making loader.bin .."
	$(shell mkdir -p ./bin)
	nasm -I ${DIR_BOOT_INC}/ -o ${BIN}/loader.bin ${DIR_BOOT}/loader.S 

${BIN}/kernel/print.o : ${DIR_LIB}/kernel/print.S
	@echo "making print.o .."
	$(shell mkdir -p ./bin/kernel)
	nasm -f elf -o ${BIN}/kernel/print.o ${DIR_LIB}/kernel/print.S

${BIN}/main.o : ${DIR_KERNEL}/main.c
	@echo "making main.o .."
	$(shell mkdir -p ./bin)
	gcc -I ${DIR_LIB}/kernel/ -c -o ${BIN}/main.o ${DIR_KERNEL}/main.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99

${BIN}/kernel.bin : ${BIN}/main.o ${BIN}/kernel/print.o
	@echo "making kernel.bin .."
	ld -Ttext 0xC0001500 -e _start -o ${BIN}/kernel.bin	\
		 -m elf_i386 ${BIN}/main.o ${BIN}/kernel/print.o

.PHONY : clean

clean :
	rm -rf ./bin

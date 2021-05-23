DIR_BOOT = ./source/boot
DIR_KERNEL = ./source/kernel
DIR_DEVICE = ./source/device
DIR_LIB = ./source/lib

## boot related
SRC_BOOT = ${DIR_BOOT}/*.S
DIR_BOOT_INC = ${DIR_BOOT}/include

BIN = ./bin
DIR_DISK = ./disk

${DIR_DISK}/bochsrc.disk : ${BIN}/kernel.bin ${BIN}/mbr.bin ${BIN}/loader.bin
	@echo "writing to disk .."
	$(shell rm ./disk/hd60M.img)
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
	gcc -I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ \
		-c -o ${BIN}/main.o ${DIR_KERNEL}/main.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector

${BIN}/kernel.o : ${DIR_KERNEL}/kernel.S
	@echo "making kernel.o .."
	$(shell mkdir -p ./bin)
	nasm -f elf -o ${BIN}/kernel.o ${DIR_KERNEL}/kernel.S

${BIN}/interrupt.o : ${DIR_KERNEL}/interrupt.c
	@echo "making interrupt.o .."
	$(shell mkdir -p ./bin)
	gcc -I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ \
		-c -o ${BIN}/interrupt.o ${DIR_KERNEL}/interrupt.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector

${BIN}/init.o : ${DIR_KERNEL}/init.c
	@echo "making init.o .."
	$(shell mkdir -p ./bin)
	gcc -I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ \
		-c -o ${BIN}/init.o ${DIR_KERNEL}/init.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector

${BIN}/timer.o : ${DIR_DEVICE}/timer.c
	@echo "making init.o .."
	$(shell mkdir -p ./bin)
	gcc -I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ \
		-c -o ${BIN}/timer.o ${DIR_DEVICE}/timer.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector

${BIN}/debug.o : ${DIR_KERNEL}/debug.c
	@echo "making init.o .."
	$(shell mkdir -p ./bin)
	gcc -I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ \
		-c -o ${BIN}/debug.o ${DIR_KERNEL}/debug.c \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector

${BIN}/kernel.bin : ${BIN}/main.o ${BIN}/kernel/print.o ${BIN}/kernel.o ${BIN}/interrupt.o ${BIN}/init.o ${BIN}/timer.o ${BIN}/debug.o
	@echo "making kernel.bin .."
	ld -Ttext 0xC0001500 -e _start -o ${BIN}/kernel.bin	\
		 -m elf_i386 $^

.PHONY : clean

clean :
	rm -rf ${BIN}/

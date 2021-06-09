DIR_BOOT = ./source/boot
DIR_KERNEL = ./source/kernel
DIR_DEVICE = ./source/device
DIR_THREAD = ./source/thread
DIR_LIB = ./source/lib
DIR_USERPROG = ./source/userprog

## boot related
SRC_BOOT = ${DIR_BOOT}/*.S
DIR_BOOT_INC = ${DIR_BOOT}/include

BIN = ./bin
DIR_DISK = ./disk

INCLUDE_PATH=-I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ -I ${DIR_THREAD}/ -I ${DIR_USERPROG}/

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

${BIN}/kernel/print_asm.o : ${DIR_LIB}/kernel/print_asm.S
	@echo "making print_asm.o .."
	$(shell mkdir -p ./bin/kernel)
	nasm -f elf -o $@ $^

${BIN}/switch.o : ${DIR_THREAD}/switch.S
	@echo "making switch.bin .."
	$(shell mkdir -p ./bin)
	nasm -f elf -o $@ $^

${BIN}/main.o : ${DIR_KERNEL}/main.c
	@echo "making main.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/kernel.o : ${DIR_KERNEL}/kernel.S
	@echo "making kernel.o .."
	$(shell mkdir -p ./bin)
	nasm -f elf -o ${BIN}/kernel.o ${DIR_KERNEL}/kernel.S

${BIN}/interrupt.o : ${DIR_KERNEL}/interrupt.c
	@echo "making interrupt.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/init.o : ${DIR_KERNEL}/init.c
	@echo "making init.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/timer.o : ${DIR_DEVICE}/timer.c
	@echo "making timer.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/keyboard.o : ${DIR_DEVICE}/keyboard.c
	@echo "making keyboard.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/console.o : ${DIR_DEVICE}/console.c
	@echo "making console.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/debug.o : ${DIR_KERNEL}/debug.c
	@echo "making debug.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/string.o : ${DIR_LIB}/string.c
	@echo "making string.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^ 

${BIN}/bitmap.o : ${DIR_LIB}/kernel/bitmap.c
	@echo "making bitmap.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/memory.o : ${DIR_KERNEL}/memory.c
	@echo "making memory.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/kernel/print.o : ${DIR_LIB}/kernel/print.c ${BIN}/kernel/print_asm.o
	@echo "making print.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ ${DIR_LIB}/kernel/print.c

${BIN}/thread.o : ${DIR_THREAD}/thread.c
	@echo "making thread.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/sync.o : ${DIR_THREAD}/sync.c
	@echo "making sync.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/kernel/list.o : ${DIR_LIB}/kernel/list.c
	@echo "making list.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/ioqueue.o : ${DIR_DEVICE}/ioqueue.c
	@echo "making ioqueue.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/tss.o : ${DIR_USERPROG}/tss.c
	@echo "making tss.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/process.o : ${DIR_USERPROG}/process.c
	@echo "making tss.o .."
	$(shell mkdir -p ./bin)
	gcc ${INCLUDE_PATH} \
		-m32 -fno-asynchronous-unwind-tables -std=c99 -fno-builtin -fno-stack-protector \
		-c -o $@ $^

${BIN}/kernel.bin : ${BIN}/main.o ${BIN}/kernel/print.o ${BIN}/kernel/print_asm.o ${BIN}/kernel.o 		\
   ${BIN}/interrupt.o ${BIN}/init.o ${BIN}/timer.o ${BIN}/debug.o ${BIN}/string.o ${BIN}/memory.o 		\
   ${BIN}/bitmap.o ${BIN}/thread.o ${BIN}/kernel/list.o ${BIN}/switch.o ${BIN}/sync.o ${BIN}/console.o	\
   ${BIN}/keyboard.o ${BIN}/ioqueue.o ${BIN}/tss.o ${BIN}/process.o
	@echo "making kernel.bin .."
	ld -Ttext 0xC0001500 -e _start -o $@ \
		 -m elf_i386 $^

.PHONY : clean

clean :
	rm -rf ${BIN}/

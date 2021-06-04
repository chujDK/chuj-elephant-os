DIR_BOOT = ./source/boot
DIR_KERNEL = ./source/kernel
DIR_DEVICE = ./source/device
DIR_THREAD = ./source/thread
DIR_LIB = ./source/lib

## boot related
SRC_BOOT = ${DIR_BOOT}/*.S
DIR_BOOT_INC = ${DIR_BOOT}/include

BIN = ./bin
DIR_DISK = ./disk

INCLUDE_PATH=-I ${DIR_LIB}/kernel/ -I ${DIR_LIB}/ -I ${DIR_KERNEL}/ -I ${DIR_DEVICE}/ -I ${DIR_THREAD}/

${DIR_DISK}/bochsrc.disk : ${BIN}/mbr.bin ${BIN}/loader.bin
	@echo "writing to disk .."
	$(shell rm ./disk/hd60M.img)
	bximage -mode=create -hd=60M -sectsize=512 -q ${DIR_DISK}/hd60M.img
	dd if=${BIN}/mbr.bin of=${DIR_DISK}/hd60M.img bs=512 count=1 seek=0 conv=notrunc
	dd if=${BIN}/loader.bin of=${DIR_DISK}/hd60M.img bs=512 count=4 seek=2 conv=notrunc

${BIN}/mbr.bin : ${DIR_BOOT}/mbr.S
	@echo "making mbr.bin .."
	$(shell mkdir -p ./bin)
	nasm -I ${DIR_BOOT_INC}/ -o ${BIN}/mbr.bin ${DIR_BOOT}/mbr.S 

${BIN}/loader.bin : ${DIR_BOOT}/loader.S
	@echo "making loader.bin .."
	$(shell mkdir -p ./bin)
	nasm -I ${DIR_BOOT_INC}/ -o ${BIN}/loader.bin ${DIR_BOOT}/loader.S 

.PHONY : clean

clean :
	rm -rf ${BIN}/
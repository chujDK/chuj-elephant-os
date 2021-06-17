#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "stdint.h"
#include "global.h"
#include "bitmap.h"
#include "sync.h"
#include "list.h"

struct ide_partition
{
	uint32_t start_lbc;
	uint32_t sector_cnt;
	struct disk* disk_belongs;
	struct list_elem part_tag;
	char name[8];	
	struct super_block* super_blcok;
	struct bitmap block_bitmap;
	struct bitmap inode_bitmap;
	struct list open_inodes;
};

struct disk
{
	char name[8];
	struct ide_channel* channel_belongs; 
	uint8_t dev_no;							// master disk or slave disk
	struct ide_partition prim_parts[4]; 	// stores the main partition 
	struct ide_partition logic_parts[8];
};

struct ide_channel
{
	char name[8];
	uint16_t 
};



#endif
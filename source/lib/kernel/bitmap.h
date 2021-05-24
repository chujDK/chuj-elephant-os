#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H
#include "global.h"
#define BITMAP_MASK 1 /* 1: this physic page had been mapped */
struct bitmap
{
    size_t bitmap_bytes_len;
    uint8_t* bits;
};

void BitmapInit(struct bitmap* btmp);
bool BitmapTestBit(struct bitmap* btmp, size_t bit_idx);
/* return 1 when the bit is 1 */
int BitmapScan(struct bitmap* btmp, size_t cnt);
void BitmapSetBit(struct bitmap* btmp, size_t bit_idx, int8_t value);
#endif

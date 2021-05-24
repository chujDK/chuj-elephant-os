#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void BitmapInit(struct bitmap* btmp)
{
    memset(btmp->bits, 0, btmp->bitmap_bytes_len);
    return;
}

int BitmapTestBit(struct bitmap* btmp, size_t bit_idx)
{
    size_t byte_idx = bit_idx / 8;
    size_t bit_order = bit_idx % 8;
    return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_order));
}

int BitmapScan(struct bitmap* btmp, size_t cnt)
{
    size_t byte_idx = 0;
    while (btmp->bits[byte_idx] == 0xFF && byte_idx < btmp->bitmap_bytes_len)
    {
        byte_idx++;
    }
    ASSERT(byte_idx < btmp->bitmap_bytes_len); /* this means all physic page had been mapped */
    if (byte_idx >= btmp->bitmap_bytes_len)
    {
        return -1;
    }
    
    size_t bit_idx = 0;
    while ((uint8_t)(BITMAP_MASK << bit_idx) & btmp->bits[byte_idx])
    {
        ++bit_idx;
    }

    int bit_idx_start = byte_idx * 8 + bit_idx;
    if (cnt == 1)
    {
        return bit_idx_start;
    }

    size_t bits_left = btmp->bitmap_bytes_len * 8 - bit_idx_start;
    size_t next_bit = bit_idx_start + 1;
    size_t avl_bits_count = 1;

    while (bits_left-- > 0)
    {
        if (!(BitmapTestBit(btmp, next_bit)))
        {
            avl_bits_count++;
        }
        else
        {
            avl_bits_count = 0;
        }
        if (avl_bits_count == cnt)
        {
            return next_bit - cnt + 1;
            break;
        }
        next_bit++;
    }
    return -1;
}

void BitmapSetBit(struct bitmap* btmp, size_t bit_idx, int8_t value)
{
    ASSERT((value == 0) || (value == 1));
    size_t byte_idx = bit_idx / 8;
    size_t bit_order = bit_idx % 8;
    if (value)
    {
       btmp->bits[byte_idx] |= (BITMAP_MASK << bit_order);
    }
    else
    {
       btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_order);
    }
}


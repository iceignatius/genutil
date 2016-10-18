/*
 * DES Bit Type and Operation
 */
#ifndef _GEN_DES_BIT_H_
#define _GEN_DES_BIT_H_

#include "../type.h"
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int sbit_t;  // Use this type just like a single bit.
                     // And note that the value of a sbit_t can be only ONE or ZERO.

INLINE void sbit_separate(sbit_t bits[8], byte_t byte)
{
    bits[0] = ( byte & ( 0x80 >> 0 ) ) >> 7;
    bits[1] = ( byte & ( 0x80 >> 1 ) ) >> 6;
    bits[2] = ( byte & ( 0x80 >> 2 ) ) >> 5;
    bits[3] = ( byte & ( 0x80 >> 3 ) ) >> 4;
    bits[4] = ( byte & ( 0x80 >> 4 ) ) >> 3;
    bits[5] = ( byte & ( 0x80 >> 5 ) ) >> 2;
    bits[6] = ( byte & ( 0x80 >> 6 ) ) >> 1;
    bits[7] = ( byte & ( 0x80 >> 7 ) ) >> 0;
}

INLINE byte_t sbit_combine(const sbit_t bits[8])
{
    return ( bits[0] << 7 ) |
           ( bits[1] << 6 ) |
           ( bits[2] << 5 ) |
           ( bits[3] << 4 ) |
           ( bits[4] << 3 ) |
           ( bits[5] << 2 ) |
           ( bits[6] << 1 ) |
           ( bits[7] << 0 );
}

INLINE void sbit_permute(sbit_t dest[], unsigned destcnt, const sbit_t src[], const int sort_table[])
{
    unsigned i;
    for(i=0; i<destcnt; ++i)
        dest[i] = src[sort_table[i]];
}

INLINE void sbit_shift_left_1(sbit_t bits[], unsigned cnt)
{
    sbit_t   temp;
    unsigned i;

    temp = bits[0];
    for(i=1; i<cnt; ++i)
        bits[i-1] = bits[i];
    bits[cnt-1] = temp;
}

INLINE void sbit_shift_left_n(sbit_t bits[], unsigned cnt, unsigned shiftcnt)
{
    while( shiftcnt-- )
        sbit_shift_left_1(bits, cnt);
}

INLINE void sbit_xor(sbit_t dest[], const sbit_t src1[], const sbit_t src2[], unsigned cnt)
{
    unsigned i;
    for(i=0; i<cnt; ++i)
        dest[i] = ( src1[i] == src2[i] )?( 0 ):( 1 );
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

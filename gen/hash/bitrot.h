/*
 * PRIVATE USAGE
 */
/**
 * @brief     Bits rotation.
 * @details   Rotate bits data.
 * @author    王文佑
 * @dare      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_BITROT_H_
#define _GEN_BITROT_H_

#include <stddef.h>
#include <stdint.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

INLINE uint32_t bitrot32_left (uint32_t x, size_t n){ return ( x << n ) | ( x >> ( 32 - n ) ); }
INLINE uint32_t bitrot32_right(uint32_t x, size_t n){ return ( x >> n ) | ( x << ( 32 - n ) ); }
INLINE uint64_t bitrot64_left (uint64_t x, size_t n){ return ( x << n ) | ( x >> ( 64 - n ) ); }
INLINE uint64_t bitrot64_right(uint64_t x, size_t n){ return ( x >> n ) | ( x << ( 64 - n ) ); }

#ifdef __cplusplus
} extern "C"
#endif

#endif

/**
 * @file
 * @brief     Endian format.
 * @details   Check and translate about endian format.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_ENDIAN_H_
#define _GEN_ENDIAN_H_

#include "type.h"
#include "inline.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @name Endian format - inquiry
/// @{

INLINE bool endian_is_little_endian(void)
{
#if defined(__GNUC__) && defined(__BYTE_ORDER__)
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#else
    union endian_test
    {
        uint8_t  bytes[4];
        uint32_t value;
    };

    static const union endian_test test = {{ 0xFF, 0x00, 0x00, 0x00 }};
    return ( test.value & 0xFF ) != 0;
#endif
}

INLINE bool endian_is_big_endian(void)
{
#if defined(__GNUC__) && defined(__BYTE_ORDER__)
    return __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__;
#else
    union endian_test
    {
        uint8_t  bytes[4];
        uint32_t value;
    };

    static const union endian_test test = {{ 0xFF, 0x00, 0x00, 0x00 }};
    return ( test.value & 0xFF ) == 0;
#endif
}

/// @}

/// @name Endian swap - integer
/// @{

INLINE uint16_t endian_swap_16(uint16_t val)
{
    return ( val << 8 )|( val >> 8 );
}

INLINE uint32_t endian_swap_32(uint32_t val)
{
    val = ( ( val << 8 )&0xFF00FF00 )|( ( val >> 8 )&0x00FF00FF );
    return ( val << 16 )|( val >> 16 );
}

INLINE uint64_t endian_swap_64(uint64_t val)
{
    val = ( ( val <<  8 )&0xFF00FF00FF00FF00LL )|( ( val >>  8 )&0x00FF00FF00FF00FFLL );
    val = ( ( val << 16 )&0xFFFF0000FFFF0000LL )|( ( val >> 16 )&0x0000FFFF0000FFFFLL );
    return ( val << 32 )|( val >> 32 );
}

/// @}

/// @name Endian swap - floating point
/// @{

INLINE float32_t endian_swap_32f(float32_t val)
{
    union{
        uint32_t  i;
        float32_t f;
    } temp;

    temp.f = val;
    temp.i = endian_swap_32(temp.i);

    return temp.f;
}

INLINE float64_t endian_swap_64f(float64_t val)
{
    union{
        uint64_t  i;
        float64_t f;
    } temp;

    temp.f = val;
    temp.i = endian_swap_64(temp.i);

    return temp.f;
}

/// @}

/// @name Endian swap - local format
/// @{

INLINE uint16_t endian_local_to_be_16(uint16_t val){ return endian_is_little_endian() ? endian_swap_16(val) : val; }
INLINE uint32_t endian_local_to_be_32(uint32_t val){ return endian_is_little_endian() ? endian_swap_32(val) : val; }
INLINE uint64_t endian_local_to_be_64(uint64_t val){ return endian_is_little_endian() ? endian_swap_64(val) : val; }
INLINE uint16_t endian_local_to_le_16(uint16_t val){ return endian_is_big_endian() ? endian_swap_16(val) : val; }
INLINE uint32_t endian_local_to_le_32(uint32_t val){ return endian_is_big_endian() ? endian_swap_32(val) : val; }
INLINE uint64_t endian_local_to_le_64(uint64_t val){ return endian_is_big_endian() ? endian_swap_64(val) : val; }

INLINE uint16_t endian_be_to_local_16(uint16_t val){ return endian_is_little_endian() ? endian_swap_16(val) : val; }
INLINE uint32_t endian_be_to_local_32(uint32_t val){ return endian_is_little_endian() ? endian_swap_32(val) : val; }
INLINE uint64_t endian_be_to_local_64(uint64_t val){ return endian_is_little_endian() ? endian_swap_64(val) : val; }
INLINE uint16_t endian_le_to_local_16(uint16_t val){ return endian_is_big_endian() ? endian_swap_16(val) : val; }
INLINE uint32_t endian_le_to_local_32(uint32_t val){ return endian_is_big_endian() ? endian_swap_32(val) : val; }
INLINE uint64_t endian_le_to_local_64(uint64_t val){ return endian_is_big_endian() ? endian_swap_64(val) : val; }

INLINE float32_t endian_local_to_be_32f(float32_t val){ return endian_is_little_endian() ? endian_swap_32f(val) : val; }
INLINE float64_t endian_local_to_be_64f(float64_t val){ return endian_is_little_endian() ? endian_swap_64f(val) : val; }
INLINE float32_t endian_local_to_le_32f(float32_t val){ return endian_is_big_endian() ? endian_swap_32f(val) : val; }
INLINE float64_t endian_local_to_le_64f(float64_t val){ return endian_is_big_endian() ? endian_swap_64f(val) : val; }

INLINE float32_t endian_be_to_local_32f(float32_t val){ return endian_is_little_endian() ? endian_swap_32f(val) : val; }
INLINE float64_t endian_be_to_local_64f(float64_t val){ return endian_is_little_endian() ? endian_swap_64f(val) : val; }
INLINE float32_t endian_le_to_local_32f(float32_t val){ return endian_is_big_endian() ? endian_swap_32f(val) : val; }
INLINE float64_t endian_le_to_local_64f(float64_t val){ return endian_is_big_endian() ? endian_swap_64f(val) : val; }

/// @}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

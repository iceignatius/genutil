/**
 * @file
 * @brief     Bits operation.
 * @details   Advanced tools about bits operation.
 * @author    王文佑
 * @date      2015.08.10
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_BITS_H_
#define _GEN_BITS_H_

#include <stdint.h>
#include <limits.h>
#include "inline.h"

/**
 * @name Bit set
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

INLINE
int bits_get_bit(const void *bits, unsigned index)
{
    /// Check if the specified bit has set to 1.
    return ( ((uint8_t*)bits)[ index >> 3 ] & ( 0x80 >> ( index & 0x07 ) ) )?( 1 ):( 0 );
}

INLINE
void bits_set_bit(void *bits, unsigned index)
{
    /// Set specified bit to 1.
    ((uint8_t*)bits)[ index >> 3 ] |= ( 0x80 >> ( index & 0x07 ) );
}

INLINE
void bits_clear_bit(void *bits, unsigned index)
{
    /// Set specified bit to 0.
    ((uint8_t*)bits)[ index >> 3 ] &= ~( 0x80 >> ( index & 0x07 ) );
}

INLINE
void bits_switch_bit(void *bits, unsigned index, int value)
{
    /// Set specified bit to 1 or 0.
    /// @note The @a value can be ONE or ZERO only.
    ( value )?( bits_set_bit(bits, index) ):( bits_clear_bit(bits, index) );
}

#ifdef __cplusplus
}  // extern "C"
#endif

/**
 * @}
 */

/**
 * @name Bits count
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

INLINE
int bits_count_bits_64(uint64_t value)
{
    /// Count non-zero bits of a 64-bits value.
#if defined(__GNUC__) && ULLONG_MAX >= 0xFFFFFFFFFFFFFFFFLL
    return __builtin_popcountll(value);
#else
    value = ( value & 0x5555555555555555LL ) + ( ( value >>  1 ) & 0x5555555555555555LL );
    value = ( value & 0x3333333333333333LL ) + ( ( value >>  2 ) & 0x3333333333333333LL );
    value = ( value & 0x0F0F0F0F0F0F0F0FLL ) + ( ( value >>  4 ) & 0x0F0F0F0F0F0F0F0FLL );
    value = ( value & 0x00FF00FF00FF00FFLL ) + ( ( value >>  8 ) & 0x00FF00FF00FF00FFLL );
    value = ( value & 0x0000FFFF0000FFFFLL ) + ( ( value >> 16 ) & 0x0000FFFF0000FFFFLL );
    value = ( value & 0x00000000FFFFFFFFLL ) + ( ( value >> 32 ) & 0x00000000FFFFFFFFLL );
    return value;
#endif
}

INLINE
int bits_count_bits_32(uint32_t value)
{
    /// Count non-zero bits of a 32-bits value.
#if defined(__GNUC__) && UINT_MAX >= 0xFFFFFFFFLL
    return __builtin_popcount(value);
#else
    value = ( value & 0x55555555 ) + ( ( value >>  1 ) & 0x55555555 );
    value = ( value & 0x33333333 ) + ( ( value >>  2 ) & 0x33333333 );
    value = ( value & 0x0F0F0F0F ) + ( ( value >>  4 ) & 0x0F0F0F0F );
    value = ( value & 0x00FF00FF ) + ( ( value >>  8 ) & 0x00FF00FF );
    value = ( value & 0x0000FFFF ) + ( ( value >> 16 ) & 0x0000FFFF );
    return value;
#endif
}

INLINE
int bits_count_bits_16(uint16_t value)
{
    /// Count non-zero bits of a 16-bits value.
#if defined(__GNUC__)
    return __builtin_popcount(value);
#else
    value = ( value & 0x5555 ) + ( ( value >> 1 ) & 0x5555 );
    value = ( value & 0x3333 ) + ( ( value >> 2 ) & 0x3333 );
    value = ( value & 0x0F0F ) + ( ( value >> 4 ) & 0x0F0F );
    value = ( value & 0x00FF ) + ( ( value >> 8 ) & 0x00FF );
    return value;
#endif
}

INLINE
int bits_count_bits_8(uint8_t value)
{
    /// Count non-zero bits of a 8-bits value.
#if defined(__GNUC__)
    return __builtin_popcount(value);
#else
    value = ( value & 0x55 ) + ( ( value >> 1 ) & 0x55 );
    value = ( value & 0x33 ) + ( ( value >> 2 ) & 0x33 );
    value = ( value & 0x0F ) + ( ( value >> 4 ) & 0x0F );
    return value;
#endif
}

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

    inline int bits_count_bits(uint64_t value) { return bits_count_bits_64(value); }  ///< @overload ::bits_count_bits_64
    inline int bits_count_bits(uint32_t value) { return bits_count_bits_32(value); }  ///< @overload ::bits_count_bits_32
    inline int bits_count_bits(uint16_t value) { return bits_count_bits_16(value); }  ///< @overload ::bits_count_bits_16
    inline int bits_count_bits(uint8_t  value) { return bits_count_bits_8 (value); }  ///< @overload ::bits_count_bits_8
    inline int bits_count_bits(int64_t  value) { return bits_count_bits_64(value); }  ///< @overload ::bits_count_bits_64
    inline int bits_count_bits(int32_t  value) { return bits_count_bits_32(value); }  ///< @overload ::bits_count_bits_32
    inline int bits_count_bits(int16_t  value) { return bits_count_bits_16(value); }  ///< @overload ::bits_count_bits_16
    inline int bits_count_bits(int8_t   value) { return bits_count_bits_8 (value); }  ///< @overload ::bits_count_bits_8

#elif __STDC_VERSION__ >= 201112L

    /// @overload ::bits_count_bits_#
    #define bits_count_bits(value) _Generic((value), uint64_t : bits_count_bits_64, \
                                                     uint32_t : bits_count_bits_32, \
                                                     uint16_t : bits_count_bits_16, \
                                                     uint8_t  : bits_count_bits_8 , \
                                                     int64_t  : bits_count_bits_64, \
                                                     int32_t  : bits_count_bits_32, \
                                                     int16_t  : bits_count_bits_16, \
                                                     int8_t   : bits_count_bits_8 , \
                                                     default  : bits_count_bits_64) (value)

#else

    /// @overload ::bits_count_bits_#
    #define bits_count_bits(value) bits_count_bits_64(value)

#endif

/**
 * @}
 */

/**
 * @name Leading zeros
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

INLINE
int bits_count_leading_zeros_64(uint64_t value)
{
    /// Count leading zero bits of a 64-bits value.
#if defined(__GNUC__) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFFLL
    return __builtin_clzll(value);
#elif defined(__GNUC__) && ULONG_MAX == 0xFFFFFFFFFFFFFFFFLL
    return __builtin_clzl(value);
#else
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return bits_count_bits_64(~value);
#endif
}

INLINE
int bits_count_leading_zeros_32(uint32_t value)
{
    /// Count leading zero bits of a 32-bits value.
#if defined(__GNUC__) && ULONG_MAX == 0xFFFFFFFFL
    return __builtin_clzl(value);
#elif defined(__GNUC__) && UINT_MAX == 0xFFFFFFFFL
    return __builtin_clz(value);
#else
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return bits_count_bits_32(~value);
#endif
}

INLINE
int bits_count_leading_zeros_16(uint16_t value)
{
    /// Count leading zero bits of a 16-bits value.
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    return bits_count_bits_16(~value);
}

INLINE
int bits_count_leading_zeros_8(uint8_t value)
{
    /// Count leading zero bits of a 8-bits value.
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    return bits_count_bits_8(~value);
}

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

    inline int bits_count_leading_zeros(uint64_t value) { return bits_count_leading_zeros_64(value); }  ///< @overload ::bits_count_leading_zeros_64
    inline int bits_count_leading_zeros(uint32_t value) { return bits_count_leading_zeros_32(value); }  ///< @overload ::bits_count_leading_zeros_32
    inline int bits_count_leading_zeros(uint16_t value) { return bits_count_leading_zeros_16(value); }  ///< @overload ::bits_count_leading_zeros_16
    inline int bits_count_leading_zeros(uint8_t  value) { return bits_count_leading_zeros_8 (value); }  ///< @overload ::bits_count_leading_zeros_8
    inline int bits_count_leading_zeros(int64_t  value) { return bits_count_leading_zeros_64(value); }  ///< @overload ::bits_count_leading_zeros_64
    inline int bits_count_leading_zeros(int32_t  value) { return bits_count_leading_zeros_32(value); }  ///< @overload ::bits_count_leading_zeros_32
    inline int bits_count_leading_zeros(int16_t  value) { return bits_count_leading_zeros_16(value); }  ///< @overload ::bits_count_leading_zeros_16
    inline int bits_count_leading_zeros(int8_t   value) { return bits_count_leading_zeros_8 (value); }  ///< @overload ::bits_count_leading_zeros_8

#elif __STDC_VERSION__ >= 201112L

    /// @overload ::bits_count_leading_zeros_#
    #define bits_count_leading_zeros(value) _Generic((value), uint64_t : bits_count_leading_zeros_64, \
                                                              uint32_t : bits_count_leading_zeros_32, \
                                                              uint16_t : bits_count_leading_zeros_16, \
                                                              uint8_t  : bits_count_leading_zeros_8 , \
                                                              int64_t  : bits_count_leading_zeros_64, \
                                                              int32_t  : bits_count_leading_zeros_32, \
                                                              int16_t  : bits_count_leading_zeros_16, \
                                                              int8_t   : bits_count_leading_zeros_8 , \
                                                              default  : bits_count_leading_zeros_64) (value)

#else

    /// @overload ::bits_count_leading_zeros_#
    #define bits_count_leading_zeros(value) bits_count_leading_zeros_64(value)

#endif

/**
 * @}
 */

/**
 * @name Trailing zeros
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

INLINE
int bits_count_trailing_zeros_64(uint64_t value)
{
    /// Count trailing zero bits of a 64-bits value.
#if defined(__GNUC__) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFFLL
    return __builtin_ctzll(value);
#elif defined(__GNUC__) && ULONG_MAX == 0xFFFFFFFFFFFFFFFFLL
    return __builtin_ctzl(value);
#else
    return bits_count_bits_64( ( value & -value ) - 1 );
#endif
}

INLINE
int bits_count_trailing_zeros_32(uint32_t value)
{
    /// Count trailing zero bits of a 32-bits value.
#if defined(__GNUC__) && ULONG_MAX == 0xFFFFFFFFL
    return __builtin_ctzl(value);
#elif defined(__GNUC__) && UINT_MAX == 0xFFFFFFFFL
    return __builtin_ctz(value);
#else
    return bits_count_bits_32( ( value & -value ) - 1 );
#endif
}

INLINE
int bits_count_trailing_zeros_16(uint16_t value)
{
    /// Count trailing zero bits of a 16-bits value.
#if defined(__GNUC__)
    return __builtin_ctz(value);
#else
    return bits_count_bits_16( ( value & -value ) - 1 );
#endif
}

INLINE
int bits_count_trailing_zeros_8(uint8_t value)
{
    /// Count trailing zero bits of a 8-bits value.
#if defined(__GNUC__)
    return __builtin_ctz(value);
#else
    return bits_count_bits_8( ( value & -value ) - 1 );
#endif
}

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

    inline int bits_count_trailing_zeros(uint64_t value) { return bits_count_trailing_zeros_64(value); }  ///< @overload ::bits_count_trailing_zeros_64
    inline int bits_count_trailing_zeros(uint32_t value) { return bits_count_trailing_zeros_32(value); }  ///< @overload ::bits_count_trailing_zeros_32
    inline int bits_count_trailing_zeros(uint16_t value) { return bits_count_trailing_zeros_16(value); }  ///< @overload ::bits_count_trailing_zeros_16
    inline int bits_count_trailing_zeros(uint8_t  value) { return bits_count_trailing_zeros_8 (value); }  ///< @overload ::bits_count_trailing_zeros_8
    inline int bits_count_trailing_zeros(int64_t  value) { return bits_count_trailing_zeros_64(value); }  ///< @overload ::bits_count_trailing_zeros_64
    inline int bits_count_trailing_zeros(int32_t  value) { return bits_count_trailing_zeros_32(value); }  ///< @overload ::bits_count_trailing_zeros_32
    inline int bits_count_trailing_zeros(int16_t  value) { return bits_count_trailing_zeros_16(value); }  ///< @overload ::bits_count_trailing_zeros_16
    inline int bits_count_trailing_zeros(int8_t   value) { return bits_count_trailing_zeros_8 (value); }  ///< @overload ::bits_count_trailing_zeros_8

#elif __STDC_VERSION__ >= 201112L

    /// @overload ::bits_count_trailing_zeros_#
    #define bits_count_trailing_zeros(value) _Generic((value), uint64_t : bits_count_trailing_zeros_64, \
                                                               uint32_t : bits_count_trailing_zeros_32, \
                                                               uint16_t : bits_count_trailing_zeros_16, \
                                                               uint8_t  : bits_count_trailing_zeros_8 , \
                                                               int64_t  : bits_count_trailing_zeros_64, \
                                                               int32_t  : bits_count_trailing_zeros_32, \
                                                               int16_t  : bits_count_trailing_zeros_16, \
                                                               int8_t   : bits_count_trailing_zeros_8 , \
                                                               default  : bits_count_trailing_zeros_64) (value)

#else

    /// @overload ::bits_count_trailing_zeros_#
    #define bits_count_trailing_zeros(value) bits_count_trailing_zeros_64(value)

#endif

/**
 * @}
 */

#endif

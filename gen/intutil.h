/**
 * @file
 * @brief     Integer utility.
 * @details   Some small and simple utility of integer calculation.
 * @author    王文佑
 * @date      2016.03.19
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_INTUTIL_H_
#define _GEN_INTUTIL_H_

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline
bool intutil_is_pow2(unsigned long value)
{
    /// Test is the input one of the power series of 2.
    return !( value & ( value - 1 ) );
}

static inline
bool intutil_is_pow2_ull(unsigned long long value)
{
    /// Test is the input one of the power series of 2.
    return !( value & ( value - 1 ) );
}

static inline
unsigned long intutil_ceil_pow2(unsigned long value)
{
    /// Calculate the smallest value of power series of 2 that
    /// is great equal than the specific input @a value.
    --value;

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
#if ULONG_MAX >= UINT32_MAX
    value |= value >> 16;
#endif
#if ULONG_MAX >= UINT64_MAX
    value |= value >> 32;
#endif
#if ULONG_MAX > UINT64_MAX
    #error Not support this kind of range.
#endif

    return ++value;
}

static inline
unsigned long long intutil_ceil_pow2_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_pow2.
    --value;

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
#if ULLONG_MAX >= UINT32_MAX
    value |= value >> 16;
#endif
#if ULLONG_MAX >= UINT64_MAX
    value |= value >> 32;
#endif
#if ULLONG_MAX > UINT64_MAX
    #error Not support this kind of range.
#endif

    return ++value;
}

static inline
unsigned long intutil_ceil_mul_n(unsigned long value, unsigned long n)
{
    /// Calculate the smallest value of multiple of @a n that
    /// is great equal than the specific input @a value.
    return ( value + ( n - 1 ) ) / n * n;
}

static inline
unsigned long long intutil_ceil_mul_n_ull(unsigned long long value, unsigned long long n)
{
    /// Unsigned long long version of ::intutil_ceil_mul_n.
    return ( value + ( n - 1 ) ) / n * n;
}

static inline
unsigned long intutil_ceil_mul_pow2(unsigned long value, unsigned long n)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for the multiple of series of power 2.
    /// @warning The input @a n must be one of the power series of 2,
    ///          or the behaviour bill be undefined.
    --n;
    return ( value + n ) & ~n;
}

static inline
unsigned long long intutil_ceil_mul_pow2_ull(unsigned long long value, unsigned long long n)
{
    /// Unsigned long long version of ::intutil_ceil_mul_pow2.
    --n;
    return ( value + n ) & ~n;
}

static inline
unsigned long intutil_ceil_mul_4(unsigned long value)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for multiple of 4.
    return ( value + ( 4 - 1 ) ) & ~( 4 - 1 );
}

static inline
unsigned long long intutil_ceil_mul_4_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_mul_4.
    return ( value + ( 4 - 1 ) ) & ~( 4 - 1 );
}

static inline
unsigned long intutil_ceil_mul_8(unsigned long value)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for multiple of 8.
    return ( value + ( 8 - 1 ) ) & ~( 8 - 1 );
}

static inline
unsigned long long intutil_ceil_mul_8_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_mul_8.
    return ( value + ( 8 - 1 ) ) & ~( 8 - 1 );
}

static inline
unsigned long intutil_ceil_mul_16(unsigned long value)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for multiple of 16.
    return ( value + ( 16 - 1 ) ) & ~( 16 - 1 );
}

static inline
unsigned long long intutil_ceil_mul_16_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_mul_4.
    return ( value + ( 16 - 1 ) ) & ~( 16 - 1 );
}

static inline
unsigned long intutil_ceil_mul_32(unsigned long value)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for multiple of 32.
    return ( value + ( 32 - 1 ) ) & ~( 32 - 1 );
}

static inline
unsigned long long intutil_ceil_mul_32_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_mul_4.
    return ( value + ( 32 - 1 ) ) & ~( 32 - 1 );
}

static inline
unsigned long intutil_ceil_mul_64(unsigned long value)
{
    /// Similarly to ::intutil_ceil_mul_n, but optimised for multiple of 64.
    return ( value + ( 64 - 1 ) ) & ~( 64 - 1 );
}

static inline
unsigned long long intutil_ceil_mul_64_ull(unsigned long long value)
{
    /// Unsigned long long version of ::intutil_ceil_mul_4.
    return ( value + ( 64 - 1 ) ) & ~( 64 - 1 );
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

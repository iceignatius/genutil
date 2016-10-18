/**
 * @file
 * @brief     Multiple value calculator.
 * @details   To calculate the next value of a specific multiple of value.
 * @author    王文佑
 * @date      2016.03.19
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NEXTMUL_H_
#define _GEN_NEXTMUL_H_

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline
bool is_pow2(unsigned long value)
{
    /// Test is the input one of the power series of 2.
    return !( value & ( value - 1 ) );
}

static inline
bool is_pow2_ull(unsigned long long value)
{
    /// Test is the input one of the power series of 2.
    return !( value & ( value - 1 ) );
}

static inline
unsigned long next_pow2(unsigned long value)
{
    /// Calculate the next value of power series of 2 that
    /// be great equal then the specific input @a value.
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
unsigned long long next_pow2_ull(unsigned long long value)
{
    /// Unsigned long long version of ::next_pow2.
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
unsigned long next_mul_n(unsigned long value, unsigned long n)
{
    /// Calculate the next multiple value of @a n that
    /// be great equal then the specific input @a value.
    return ( value + ( n - 1 ) ) / n * n;
}

static inline
unsigned long long next_mul_n_ull(unsigned long long value, unsigned long long n)
{
    /// Unsigned long long version of ::next_mul_n.
    return ( value + ( n - 1 ) ) / n * n;
}

static inline
unsigned long next_mul_pow2(unsigned long value, unsigned long n)
{
    /// Similarly to ::next_mul_n, but optimised for series of power 2 multiple.
    /// @warning The input @a n must be one of the power series of 2,
    ///          or the behaviour bill be undefined.
    --n;
    return ( value + n ) & ~n;
}

static inline
unsigned long long next_mul_pow2_ull(unsigned long long value, unsigned long long n)
{
    /// Unsigned long long version of ::next_mul_pow2.
    --n;
    return ( value + n ) & ~n;
}

static inline
unsigned long next_mul4(unsigned long value)
{
    /// Similarly to ::next_mul_n, but optimised for multiple of 4.
    return ( value + ( 4 - 1 ) ) & ~( 4 - 1 );
}

static inline
unsigned long long next_mul4_ull(unsigned long long value)
{
    /// Unsigned long long version of ::next_mul4.
    return ( value + ( 4 - 1 ) ) & ~( 4 - 1 );
}

static inline
unsigned long next_mul8(unsigned long value)
{
    /// Similarly to ::next_mul_n, but optimised for multiple of 8.
    return ( value + ( 8 - 1 ) ) & ~( 8 - 1 );
}

static inline
unsigned long long next_mul8_ull(unsigned long long value)
{
    /// Unsigned long long version of ::next_mul8.
    return ( value + ( 8 - 1 ) ) & ~( 8 - 1 );
}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

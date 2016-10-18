/**
 * @file
 * @brief     Basic type definition
 * @details   Definition of some basic types to :
 *            @li Support different operating system with less code modify.
 *            @li Support new standard types to old compilers.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_TYPE_H_
#define _GEN_TYPE_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef NO_WCHAR
#include <wchar.h>
#endif

#include "static_assert.h"

/// nullptr
#if !defined(__cplusplus) || ( __cplusplus < 201103L )
    #ifndef nullptr
        #define nullptr NULL
    #endif
#endif

/// @name Floating point with specific size
/// @{
typedef float    float32_t;
typedef double   float64_t;
/// @}

/// @name Memory associated types
/// @{
typedef uint8_t  byte_t;
/// @}

/// @name Boolean with specific size
/// @{
typedef uint8_t  bool8_t;
typedef uint16_t bool16_t;
typedef uint32_t bool32_t;
typedef uint64_t bool64_t;
/// @}

/// @name Character type with specific size
/// @{
#if !defined(__cplusplus) || ( __cplusplus < 201103 )
typedef int16_t char16_t;
typedef int32_t char32_t;
#endif
/// @}

// Size of wchar_t
#ifndef NO_WCHAR
    #if   0x7FFFFFFF <= WCHAR_MAX && WCHAR_MAX <= 0xFFFFFFFF
        #define IGNDEF_SIZEOF_WCHAR 4
    #elif 0x7FFF     <= WCHAR_MAX && WCHAR_MAX <= 0xFFFF
        #define IGNDEF_SIZEOF_WCHAR 2
    #elif 0x7F       <= WCHAR_MAX && WCHAR_MAX <= 0xFF
        #define IGNDEF_SIZEOF_WCHAR 1
    #else
        #error No implementation on this platform!
    #endif
#endif

// Check size of types
STATIC_ASSERT( sizeof(float32_t)  == 4 );
STATIC_ASSERT( sizeof(float64_t)  == 8 );
STATIC_ASSERT( sizeof(byte_t)     == 1 );
STATIC_ASSERT( sizeof(bool8_t)    == 1 );
STATIC_ASSERT( sizeof(bool16_t)   == 2 );
STATIC_ASSERT( sizeof(bool32_t)   == 4 );
STATIC_ASSERT( sizeof(bool64_t)   == 8 );
STATIC_ASSERT( sizeof(char16_t)   == 2 );
STATIC_ASSERT( sizeof(char32_t)   == 4 );
#ifndef NO_WCHAR
    STATIC_ASSERT( sizeof(wchar_t) == IGNDEF_SIZEOF_WCHAR );
#endif

// Other type definition
#ifdef WINVER
#ifdef _WIN64
typedef int64_t key_t;
#else
typedef int32_t key_t;
#endif
#endif

#endif

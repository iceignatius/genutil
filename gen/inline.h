/**
 * @file
 * @brief     In-line decoration.
 * @details   Define inline function decoration for C language.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_INLINE_H_
#define _GEN_INLINE_H_

/// In-line decoration.

#if   defined(__cplusplus)

    // C++ Compiler
    #define INLINE inline

#elif defined(__STDC__) && defined(__STDC_VERSION__) && ( __STDC_VERSION__ >= 199901L )

    // C Compiler with C99 Version or Later
    #define INLINE static inline

#elif defined(__GNUC__)

    // GNU C Compiler
    #define INLINE static __inline__

#elif defined(_MSC_VER)

    // Microsoft Visual C++
    #define INLINE __inline

#else

    // Other compilers
    #define INLINE static

#endif

#endif

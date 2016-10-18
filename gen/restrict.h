/**
 * @file
 * @brief     Restrict decoration.
 * @details   Decoration of RESTRICT.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_RESTRICT_H_
#define _GEN_RESTRICT_H_

/// Restrict
#if   defined(__GNUC__)
    #define RESTRICT __restrict
#elif defined(_MSC_VER)
    #define RESTRICT __restrict
#else
    #define RESTRICT
#endif

#endif

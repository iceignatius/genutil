/**
 * @file
 * @brief     M.T. random number.
 * @details   Mersenne Twister random number generator.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_MTRAND_H_
#define _GEN_MTRAND_H_

#include <stddef.h>
#include <stdint.h>
#include "inline.h"
#include "restrict.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * When to use the random object ?
 *
 *     For normal use, a random object pass to these functions is not necessary,
 * and a NULL pointer is recommended to use for, and a global object will be
 * used automatically.
 *     But the random object is necessary for multi-thread usage, each thread must
 * have its own object, and initialize those objects before you use they.
 */

/// Random object - 32 bits version
typedef struct mt_rand32_t
{
    #define MTRND32_MTXSZ 624
    uint32_t  state[MTRND32_MTXSZ];
    uint32_t *end;
    uint32_t *curr;
    uint32_t *next;
    uint32_t *mm;
} mt_rand32_t;

void     mt_srand32(mt_rand32_t* RESTRICT obj, uint32_t seed);
uint32_t mt_rand32 (mt_rand32_t* RESTRICT obj);

/// Random object - 64 bits version
typedef struct mt_rand64_t
{
    #define MTRND64_MTXSZ 312
    uint64_t  state[MTRND64_MTXSZ];
    uint64_t *end;
    uint64_t *curr;
    uint64_t *next;
    uint64_t *mm;
} mt_rand64_t;

void     mt_srand64(mt_rand64_t* RESTRICT obj, uint64_t seed);
uint64_t mt_rand64 (mt_rand64_t* RESTRICT obj);

// Random Data Generator
INLINE void mt_srandbin(mt_rand64_t* RESTRICT obj, uint64_t seed){ mt_srand64(obj, seed); }  ///< Set random seed.
void        mt_randbin (mt_rand64_t* RESTRICT obj, void* RESTRICT dest, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

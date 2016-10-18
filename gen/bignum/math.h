/******************************************************************************
 * Name      : Large Number Math
 * Purpose   : Sub Module of Large Number Library : Math.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_BIGNUM_MATH_H_
#define _GEN_BIGNUM_MATH_H_

#include <stdbool.h>
#include "../inline.h"
#include "../mtrand.h"
#include "./base.h"

#ifdef __cplusplus
extern "C" {
#endif

// Basic Operator

uintbig_t uintbig_add   (const uintbig_t *a, const uintbig_t *b);
uintbig_t uintbig_sub   (const uintbig_t *a, const uintbig_t *b);
uintbig_t uintbig_mul   (const uintbig_t *a, const uintbig_t *b);
void      uintbig_divmod(const uintbig_t *a, const uintbig_t *b, uintbig_t *quo, uintbig_t *rem);  // WARNING :
                                                                                                   //   The memory pointer value of 'quo' or 'rem'
                                                                                                   //   CANNOT be the same value of 'a' or 'b'!
uintbig_t uintbig_div   (const uintbig_t *a, const uintbig_t *b);
uintbig_t uintbig_mod   (const uintbig_t *a, const uintbig_t *b);

void uintbig_plusplus  (uintbig_t *a);
void uintbig_minusminus(uintbig_t *a);

// Advanced Functions

uintbig_t uintbig_power2   (const uintbig_t *a);
uintbig_t uintbig_power    (const uintbig_t *a, const uintbig_t *b);
uintbig_t uintbig_powermod (const uintbig_t *a, const uintbig_t *b, const uintbig_t *n);
uintbig_t uintbig_sqrt     (const uintbig_t *a);
uintbig_t uintbig_gcd      (const uintbig_t *a, const uintbig_t *b);
bool      uintbig_modmulinv(const uintbig_t *a, const uintbig_t *n, uintbig_t *inv);

// Random Number and Prime Number
// Note : It is recommended to call 'uintbig_srand' before using functions below.

INLINE void uintbig_srand(mt_rand64_t *rndobj, uint64_t seed){ mt_srand64(rndobj, seed); }

uintbig_t uintbig_rand_ndig (unsigned int ndig, mt_rand64_t *rndobj);
uintbig_t uintbig_rand_range(const uintbig_t *minval, const uintbig_t *maxval, mt_rand64_t *rndobj);
bool      uintbig_prime_test(const uintbig_t *val, unsigned int times, mt_rand64_t *rndobj);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

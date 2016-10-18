/******************************************************************************
 * Name      : Large Number Basic
 * Purpose   : Sub Module of Large Number Library : Basic Operation.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_BIGNUM_BASE_H_
#define _GEN_BIGNUM_BASE_H_

#include <stdbool.h>
#include "./type.h"

#ifdef __cplusplus
extern "C" {
#endif

int  uintbig_get_digit_usage(const uint8_t digit[BIGNUM_DIGSZ]);
void uintbig_set_dig        (uintbig_t *obj, uint8_t val);

bool uintbig_is_zero      (const uintbig_t *obj);
bool uintbig_is_nan       (const uintbig_t *obj);
bool uintbig_is_equal     (const uintbig_t *obj1, const uintbig_t *obj2);
bool uintbig_is_greatthen (const uintbig_t *obj1, const uintbig_t *obj2);
bool uintbig_is_lessthen  (const uintbig_t *obj1, const uintbig_t *obj2);
bool uintbig_is_greatequal(const uintbig_t *obj1, const uintbig_t *obj2);
bool uintbig_is_lessequal (const uintbig_t *obj1, const uintbig_t *obj2);

uintbig_t uintbig_and(const uintbig_t *obj1, const uintbig_t *obj2);
uintbig_t uintbig_or (const uintbig_t *obj1, const uintbig_t *obj2);
uintbig_t uintbig_xor(const uintbig_t *obj1, const uintbig_t *obj2);

void uintbig_byteshift_left (uintbig_t *obj, unsigned int n);
void uintbig_byteshift_right(uintbig_t *obj, unsigned int n);

void uintbig_shift_left (uintbig_t *obj, unsigned int n);
void uintbig_shift_right(uintbig_t *obj, unsigned int n);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

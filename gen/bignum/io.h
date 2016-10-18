/******************************************************************************
 * Name      : Large Number IO
 * Purpose   : Sub Module of Large Number Library : Import and Export.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_BIGNUM_IO_H_
#define _GEN_BIGNUM_IO_H_

#include <stddef.h>
#include "./base.h"

#ifdef __cplusplus
extern "C" {
#endif

uintbig_t uintbig_from_bin(const void *src, size_t size);
uintbig_t uintbig_from_int(uint64_t val);
uintbig_t uintbig_from_hex(const char *str);
uint64_t  uintbig_to_int  (const uintbig_t *obj);
size_t    uintbig_to_str  (char *buf, size_t bufsz, const uintbig_t *obj);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

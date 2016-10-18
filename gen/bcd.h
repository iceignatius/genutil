/**
 * @file
 * @brief     Binary Coded Decimal (BCD) encoder and decoder.
 * @details   To encode and decode the BCD data.
 * @author    王文佑
 * @date      2014.09.02
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_BCD_H_
#define _GEN_BCD_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     bcd_encode(void *buf, size_t bufsize, uint64_t value);
uint64_t bcd_decode(const void *data, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

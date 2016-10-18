/******************************************************************************
 * Name      : Large Number Types
 * Purpose   : Sub Module of Large Number Library : Definition of Types.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_BIGNUM_TYPE_H_
#define _GEN_BIGNUM_TYPE_H_

#include <stdint.h>

#ifndef BIGNUM_DIGSZ
#define BIGNUM_DIGSZ 8
#endif

#if BIGNUM_DIGSZ % 2 == 1
#error BIGNUM_DIGSZ must be an even number
#endif

#if BIGNUM_DIGSZ < 2
#error BIGNUM_DIGSZ must be great then 2
#endif

typedef struct uintbig_t
{

    uint8_t digit[BIGNUM_DIGSZ];  // 存放數字資料的空間，使用 Little-Endian 格式。
                                  // 其中未被使用的資料空間必須設為零。

    int     ndig;                 // 數字資料空間的實際使用位元組數，在非異常狀態下此值大於等於 1；
                                  // 若為負值表示資料異常：
                                  // -1 : NAN

} uintbig_t;

#endif

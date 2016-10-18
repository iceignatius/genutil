/******************************************************************************
 * Name      : AES
 * Purpose   : AES cipher functions.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_AES_H_
#define _GEN_AES_H_

#include "../type.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)
typedef struct aes_state_t
{
    union
    {
        byte_t   byte[16];
        uint32_t word[4];
    };
} aes_state_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(aes_state_t) == 16 );

size_t aes_key_expand(aes_state_t  /*out*//*opt*/ *roundkeys,
                      const byte_t /*in*/         *key,
                      size_t       /*in*/          keysz);

#ifdef AES_DEBUG_TEST
void aes_private_debug_test(void);
#endif

void aes_state_encipher(aes_state_t       /*inout*/ *state,
                        const aes_state_t /*in   */ *rndkeys,
                        size_t            /*in   */  rndkeys_cnt);
void aes_state_decipher(aes_state_t       /*inout*/ *state,
                        const aes_state_t /*in   */ *rndkeys,
                        size_t            /*in   */  rndkeys_cnt);

/**
 * AES 任意長度資料加解密參數共同說明
 *
 * @out    : 用來接收加解密後資料的緩衝區，此緩衝區需已配置足夠的大小；
 *           若此輸入為 NULL，則函式將傳回此緩衝區所需的最小大小。
 * @in     : 加解密前資料輸入。
 * @insz   : 加解密前資料大小。此數值會被調整以使資料的大小為 AES 區塊大小的整數倍，
 *           對於無法被 AES 區塊大小整除的部份將會被：
 *           無條件進位並末尾補零(用於加密時)；
 *           或無條件捨去(用於解密時)。
 * @key    : 密鑰資料。
 * @keysz  : 密鑰長度，AES 只接受 16, 24, 32 三值之一，超過尺寸的資料將被忽略；
 *           函式將會自動挑選小於等於 keysz 的最接近數值使用。
 * @iv     : 初始化向量(Initialisation Vector)，若輸入 NULL 則函式會使用
 *           aes_cbc_get_initvec 來計算初始向量。
 * @return : 函式操作成功時回傳寫入輸出緩衝區的資料大小；
 *           當 out 為 NULL 時傳回 out 所需的緩衝區大小；
 *           其他失敗情況傳回零。
 */

size_t aes_ecb_encipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz);
size_t aes_ecb_decipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz);

void aes_cbc_get_initvec(void       /*out*/ *iv,
                         const void /*in*/  *data,
                         size_t     /*in*/   size);
size_t aes_cbc_encipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz,
                        const void /*in*//*opt*/  *iv);
size_t aes_cbc_decipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz,
                        const void /*in*//*opt*/  *iv);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

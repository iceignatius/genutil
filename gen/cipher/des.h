/******************************************************************************
 * Name      : DES
 * Purpose   : DES cipher functions.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_DES_H_
#define _GEN_DES_H_

#include "../type.h"
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct des_subkey_t
{
    byte_t data[6];
} des_subkey_t;

void des_key_expand(des_subkey_t /*out*/ subkeys[16], const byte_t /*in*/ mainkey[8]);
void tdea_key_expand(byte_t /*out*/ keys[3*8], const byte_t /*in*/ *mainkey, size_t /*in*/ mainkey_size);

#ifdef DES_DEBUG_TEST
void des_private_debug_test(void);
#endif

// DES Block Cipher

void        des_block_cipher  (byte_t /*in,out*/ block[8], const byte_t /*in*/ key[8], bool /*in*/ reverse_subkeys);
INLINE void des_block_encipher(byte_t /*in,out*/ block[8], const byte_t /*in*/ key[8]) { des_block_cipher(block, key, false); }
INLINE void des_block_decipher(byte_t /*in,out*/ block[8], const byte_t /*in*/ key[8]) { des_block_cipher(block, key, true); }

// Triple-DES (TDEA) Block Cipher

INLINE void tdea_block_encipher(byte_t /*in,out*/ block[8], const byte_t /*in*/ key[24])
{
    des_block_encipher(block, key +  0);
    des_block_decipher(block, key +  8);
    des_block_encipher(block, key + 16);
}

INLINE void tdea_block_decipher(byte_t /*in,out*/ block[8], const byte_t /*in*/ key[24])
{
    des_block_decipher(block, key + 16);
    des_block_encipher(block, key +  8);
    des_block_decipher(block, key +  0);
}

/**
 * DES 任意長度資料加解密參數共同說明
 *
 * @out    : 用來接收加解密後資料的緩衝區，此緩衝區需已配置足夠的大小；
 *           若此輸入為 NULL，則函式將傳回此緩衝區所需的最小大小。
 * @in     : 加解密前資料輸入。
 * @insz   : 加解密前資料大小。此數值會被調整以使資料的大小為 AES 區塊大小的整數倍，
 *           對於無法被 AES 區塊大小整除的部份將會被：
 *           無條件進位並末尾補零(用於加密時)；
 *           或無條件捨去(用於解密時)。
 * @key    : 密鑰資料。
 * @keysz  : 密鑰長度，DES 只接受 8、16、或 24 三者之一，超過尺寸的資料將被忽略；
 *           函式將會自動挑選小於等於 keysz 的最接近數值使用。
 * @iv     : 初始化向量(Initialisation Vector)，若輸入 NULL 則函式會使用
 *           des_cbc_get_initvec 來計算初始向量。
 * @return : 函式操作成功時回傳寫入輸出緩衝區的資料大小；
 *           當 out 為 NULL 時傳回 out 所需的緩衝區大小；
 *           其他失敗情況傳回零。
 */

size_t tdea_ecb_encipher(void       /*out*//*opt*/ *out,
                         const void /*in*/         *in,
                         size_t     /*in*/          insz,
                         const void /*in*/         *key,
                         size_t     /*in*/          keysz);
size_t tdea_ecb_decipher(void       /*out*//*opt*/ *out,
                         const void /*in*/         *in,
                         size_t     /*in*/          insz,
                         const void /*in*/         *key,
                         size_t     /*in*/          keysz);

void tdea_cbc_get_initvec(byte_t      /*out*/  iv[8],
                          const void  /*in*/  *data,
                          size_t      /*in*/   size);

size_t tdea_cbc_encipher(void         /*out*//*opt*/ *out,
                         const void   /*in*/         *in,
                         size_t       /*in*/          insz,
                         const void   /*in*/         *key,
                         size_t       /*in*/          keysz,
                         const byte_t /*in*//*opt*/   iv[8]);
size_t tdea_cbc_decipher(void         /*out*//*opt*/ *out,
                         const void   /*in*/         *in,
                         size_t       /*in*/          insz,
                         const void   /*in*/         *key,
                         size_t       /*in*/          keysz,
                         const byte_t /*in*//*opt*/   iv[8]);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

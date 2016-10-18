#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../inline.h"
#include "aes_table.h"
#include "aes.h"

//------------------------------------------------------------------------------
//---- Key Expansion -----------------------------------------------------------
//------------------------------------------------------------------------------
static
void key_calcparams(size_t /*inout*/ *keysz,
                    size_t /*out  */ *Nk,
                    size_t /*out  */ *Nr,
                    size_t /*out  */ *rndkeys_cnt)
{
    assert( Nk && Nr );

    if     ( *keysz >= 32 )
    {
        *keysz       = 32;
        *Nk          = 8;
        *Nr          = 14;
        *rndkeys_cnt = 15;
    }
    else if( *keysz >= 24 )
    {
        *keysz       = 24;
        *Nk          = 6;
        *Nr          = 12;
        *rndkeys_cnt = 13;
    }
    else if( *keysz >= 16 )
    {
        *keysz       = 16;
        *Nk          = 4;
        *Nr          = 10;
        *rndkeys_cnt = 11;
    }
    else
    {
        *keysz       = 0;
        *Nk          = 0;
        *Nr          = 0;
        *rndkeys_cnt = 0;
    }
}
//------------------------------------------------------------------------------
static
uint32_t key_rotword(uint32_t word)
{
    byte_t *arr  = (byte_t*)&word;
    byte_t  temp = arr[0];
    arr[0] = arr[1];
    arr[1] = arr[2];
    arr[2] = arr[3];
    arr[3] = temp;

    return word;
}
//------------------------------------------------------------------------------
static
uint32_t key_subword(uint32_t word)
{
    byte_t *arr  = (byte_t*)&word;
    size_t  i;

    for(i=0; i<4; ++i)
        arr[i] = aes_table_sbox[arr[i]];

    return word;
}
//------------------------------------------------------------------------------
static
uint32_t key_addword(uint32_t word1, uint32_t word2)
{
    return word1 ^ word2;
}
//------------------------------------------------------------------------------
size_t aes_key_expand(aes_state_t  /*out*//*opt*/ *roundkeys,
                      const byte_t /*in*/         *key,
                      size_t       /*in*/          keysz)
{
    /**
     * 將主密鑰展開為全部的回合密鑰
     * @roundkeys : 接收填寫所有回合密鑰的緩衝區，此緩衝區需先配置有足夠的大小；
     *              若此值輸入 NULL 則函式會傳回此緩衝區所需的最小空間。
     * @key       : 主密鑰資料
     * @keysz     : 主密鑰資料長度，AES 只接受 16, 24, 32 三值之一，超過尺寸的資料將被忽略；
     *              函式將會自動挑選小於等於 keysz 的最接近數值使用。
     * @return    : 函式處理成功返回填寫入 roundkeys 陣列的陣列元素數量；處理失敗則返回零。
     *              因為 AES 的規格影響，此函式只會返回下列數值：
     *              0  : 處理失敗
     *              11 : keysz == 16
     *              13 : keysz == 24
     *              15 : keysz == 32
     */
    size_t    Nk, Nr, rndkeys_cnt;
    uint32_t *outbuf = roundkeys->word;
    uint32_t  temp;
    size_t    i;

    if( !key ) return 0;

    // Calculate precise parameters
    key_calcparams(&keysz, &Nk, &Nr, &rndkeys_cnt);
    if( keysz == 0 ) return 0;

    // Return round key buffer size right now if necessary
    if( !outbuf ) return rndkeys_cnt;

    // Calculate round keys
    memcpy(outbuf, key, Nk<<2);
    for(i=Nk; i<((Nr+1)<<2); ++i)
    {
        temp = outbuf[i-1];
        if( i%Nk == 0 )
        {
            temp = key_rotword(temp);
            temp = key_subword(temp);
            temp = key_addword(temp, aes_table_rcon[i/Nk-1]);
        }
        else if(( Nk > 6 )&&( i%Nk == 4 ))
        {
            temp = key_subword(temp);
        }
        outbuf[i] = key_addword(outbuf[i-Nk], temp);
    }

    return rndkeys_cnt;
}
//------------------------------------------------------------------------------
//---- Private Cipher Routines -------------------------------------------------
//------------------------------------------------------------------------------
static
void state_add(aes_state_t /*inout*/ *state, const aes_state_t /*in*/ *target)
{
    size_t i;
    for(i=0; i<4; ++i)
        state->word[i] ^= target->word[i];
}
//------------------------------------------------------------------------------
static
void state_subbytes(aes_state_t /*inout*/ *state, const byte_t table[256])
{
    size_t i;
    for(i=0; i<16; ++i)
        state->byte[i] = table[ state->byte[i] ];
}
//------------------------------------------------------------------------------
static
void state_rearrange(aes_state_t /*inout*/ *state, const int table[16])
{
    aes_state_t temp = *state;
    size_t      i;

    for(i=0; i<16; ++i)
        state->byte[i] = temp.byte[ table[i] ];
}
//------------------------------------------------------------------------------
static
byte_t finite_add(byte_t a, byte_t b)
{
    /*
     * Add two numbers in a GF(2^8) finite field
     */
    return a ^ b;
}
//------------------------------------------------------------------------------
static
byte_t finite_mul(byte_t a, byte_t b)
{
    /*
     * Multiply two numbers in the GF(2^8) finite field defined
     */
    int    i;
    bool   carry;
    byte_t res = 0;

    for(i=0; i<8; ++i)
    {
        if( b & 1 )
            res ^= a;
        carry = ( a & 0x80 );
        a <<= 1;
        if( carry )
            a ^= 0x1B;  // what x^8 is modulo x^8 + x^4 + x^3 + x + 1
        b >>= 1;
    }

    return res;
}
//------------------------------------------------------------------------------
static
uint32_t state_mixword(uint32_t word)
{
    union exg
    {
        byte_t   bytes[4];
        uint32_t value;
    };

    union exg src = { .value=word };
    union exg res;

    res.bytes[0] = finite_add( finite_add( finite_mul(0x02,src.bytes[0]), finite_mul(0x03,src.bytes[1]) ), finite_add(                 src.bytes[2] ,                 src.bytes[3]  ) );
    res.bytes[1] = finite_add( finite_add(                 src.bytes[0] , finite_mul(0x02,src.bytes[1]) ), finite_add( finite_mul(0x03,src.bytes[2]),                 src.bytes[3]  ) );
    res.bytes[2] = finite_add( finite_add(                 src.bytes[0] ,                 src.bytes[1]  ), finite_add( finite_mul(0x02,src.bytes[2]), finite_mul(0x03,src.bytes[3]) ) );
    res.bytes[3] = finite_add( finite_add( finite_mul(0x03,src.bytes[0]),                 src.bytes[1]  ), finite_add(                 src.bytes[2] , finite_mul(0x02,src.bytes[3]) ) );

    return res.value;
}
//------------------------------------------------------------------------------
static
uint32_t state_mixword_inv(uint32_t word)
{
    union exg
    {
        byte_t   bytes[4];
        uint32_t value;
    };

    union exg src = { .value=word };
    union exg res;

    res.bytes[0] = finite_add( finite_add( finite_mul(0x0E,src.bytes[0]), finite_mul(0x0B,src.bytes[1]) ), finite_add( finite_mul(0x0D,src.bytes[2]), finite_mul(0x09,src.bytes[3]) ) );
    res.bytes[1] = finite_add( finite_add( finite_mul(0x09,src.bytes[0]), finite_mul(0x0E,src.bytes[1]) ), finite_add( finite_mul(0x0B,src.bytes[2]), finite_mul(0x0D,src.bytes[3]) ) );
    res.bytes[2] = finite_add( finite_add( finite_mul(0x0D,src.bytes[0]), finite_mul(0x09,src.bytes[1]) ), finite_add( finite_mul(0x0E,src.bytes[2]), finite_mul(0x0B,src.bytes[3]) ) );
    res.bytes[3] = finite_add( finite_add( finite_mul(0x0B,src.bytes[0]), finite_mul(0x0D,src.bytes[1]) ), finite_add( finite_mul(0x09,src.bytes[2]), finite_mul(0x0E,src.bytes[3]) ) );

    return res.value;
}
//------------------------------------------------------------------------------
static
void state_mixcolumn(aes_state_t /*inout*/ *state)
{
    size_t i;
    for(i=0; i<4; ++i)
        state->word[i] = state_mixword( state->word[i] );
}
//------------------------------------------------------------------------------
static
void state_mixcolumn_inv(aes_state_t /*inout*/ *state)
{
    size_t i;
    for(i=0; i<4; ++i)
        state->word[i] = state_mixword_inv( state->word[i] );
}
//------------------------------------------------------------------------------
#ifdef AES_DEBUG_TEST
void aes_private_debug_test(void)
{
    static const aes_state_t key_0        = {{{ 0x2B,0x7E,0x15,0x16, 0x28,0xAE,0xD2,0xA6, 0xAB,0xF7,0x15,0x88, 0x09,0xCF,0x4F,0x3C }}};
    static const aes_state_t block_0      = {{{ 0x32,0x43,0xF6,0xA8, 0x88,0x5A,0x30,0x8D, 0x31,0x31,0x98,0xA2, 0xE0,0x37,0x07,0x34 }}};
    static const aes_state_t block_add    = {{{ 0x19,0x3D,0xE3,0xBE, 0xA0,0xF4,0xE2,0x2B, 0x9A,0xC6,0x8D,0x2A, 0xE9,0xF8,0x48,0x08 }}};
    static const aes_state_t block_sub    = {{{ 0xD4,0x27,0x11,0xAE, 0xE0,0xBF,0x98,0xF1, 0xB8,0xB4,0x5D,0xE5, 0x1E,0x41,0x52,0x30 }}};
    static const aes_state_t block_shift  = {{{ 0xD4,0xBF,0x5D,0x30, 0xE0,0xB4,0x52,0xAE, 0xB8,0x41,0x11,0xF1, 0x1E,0x27,0x98,0xE5 }}};
    static const aes_state_t block_mixcol = {{{ 0x04,0x66,0x81,0xE5, 0xE0,0xCB,0x19,0x9A, 0x48,0xF8,0xD3,0x7A, 0x28,0x06,0x26,0x4C }}};
    aes_state_t block;
    int         res;

    // State initialize
    memcpy(&block, &block_0, sizeof(aes_state_t));

    // Add round key
    state_add(&block, &key_0);
    res = memcmp(&block, &block_add, sizeof(aes_state_t));     assert( res == 0 );

    // Sub bytes
    state_subbytes(&block, aes_table_sbox);
    state_subbytes(&block, aes_table_sbox_inv);
    res = memcmp(&block, &block_add, sizeof(aes_state_t));     assert( res == 0 );
    state_subbytes(&block, aes_table_sbox);
    res = memcmp(&block, &block_sub, sizeof(aes_state_t));     assert( res == 0 );

    // Shift rows
    state_rearrange(&block, aes_table_shiftraw);
    state_rearrange(&block, aes_table_shiftraw_inv);
    res = memcmp(&block, &block_sub, sizeof(aes_state_t));     assert( res == 0 );
    state_rearrange(&block, aes_table_shiftraw);
    res = memcmp(&block, &block_shift, sizeof(aes_state_t));   assert( res == 0 );

    // Finite operator test
    res = finite_add(0x53,0xCA);                               assert( res == 0x99 );
    res = finite_mul(0x53,0xCA);                               assert( res == 0x01 );

    // Mix column
    state_mixcolumn(&block);
    state_mixcolumn_inv(&block);
    res = memcmp(&block, &block_shift, sizeof(aes_state_t));   assert( res == 0 );
    state_mixcolumn(&block);
    res = memcmp(&block, &block_mixcol, sizeof(aes_state_t));  assert( res == 0 );
}
#endif
//------------------------------------------------------------------------------
//---- Block Unit Cipher Routine -----------------------------------------------
//------------------------------------------------------------------------------
void aes_state_encipher(aes_state_t       /*inout*/ *state,
                        const aes_state_t /*in   */ *rndkeys,
                        size_t            /*in   */  rndkeys_cnt)
{
    /**
     * 單一區塊加密
     * @state       : 輸入欲加密的資料，並傳回加密後的資料。
     * @rndkeys     : 回合密鑰陣列，此資料可由 aes_key_expand 取得。
     * @rndkeys_cnt : 回合密鑰數量，此值輸入即為 aes_key_expand 的回傳值，此參數只能為 11, 13, 或 15。
     */
    size_t Nr, i;

    assert( state && rndkeys );
    assert( rndkeys_cnt == 11 ||
            rndkeys_cnt == 13 ||
            rndkeys_cnt == 15 );

    Nr = rndkeys_cnt - 1;
    state_add(state, &rndkeys[0]);
    for(i=1; i<Nr; ++i)
    {
        state_subbytes (state, aes_table_sbox);
        state_rearrange(state, aes_table_shiftraw);
        state_mixcolumn(state);
        state_add      (state, &rndkeys[i]);
    }
    state_subbytes (state, aes_table_sbox);
    state_rearrange(state, aes_table_shiftraw);
    state_add      (state, &rndkeys[i]);
}
//------------------------------------------------------------------------------
void aes_state_decipher(aes_state_t       /*inout*/ *state,
                        const aes_state_t /*in   */ *rndkeys,
                        size_t            /*in   */  rndkeys_cnt)
{
    /**
     * 單一區塊解密
     * @state       : 輸入欲解密的資料，並傳回解密後的資料。
     * @rndkeys     : 回合密鑰陣列，此資料可由 aes_key_expand 取得。
     * @rndkeys_cnt : 回合密鑰數量，此值輸入即為 aes_key_expand 的回傳值，此參數只能為 11, 13, 或 15。
     */
    size_t Nr, i;

    assert( state && rndkeys );
    assert( rndkeys_cnt == 11 ||
            rndkeys_cnt == 13 ||
            rndkeys_cnt == 15 );

    Nr = rndkeys_cnt - 1;
    state_add(state, &rndkeys[Nr]);
    for(i=Nr-1; i>0; --i)
    {
        state_rearrange    (state, aes_table_shiftraw_inv);
        state_subbytes     (state, aes_table_sbox_inv);
        state_add          (state, &rndkeys[i]);
        state_mixcolumn_inv(state);
    }
    state_rearrange(state, aes_table_shiftraw_inv);
    state_subbytes (state, aes_table_sbox_inv);
    state_add      (state, &rndkeys[i]);
}
//------------------------------------------------------------------------------
//---- Multi Block Cipher ------------------------------------------------------
//------------------------------------------------------------------------------
INLINE
size_t get_encipher_buffer_size(size_t datasize)
{
    return ( datasize + 16 - 1 ) & ~0x0F;
}
//------------------------------------------------------------------------------
INLINE
size_t get_decipher_buffer_size(size_t datasize)
{
    return datasize & ~0x0F;
}
//------------------------------------------------------------------------------
size_t aes_ecb_encipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz)
{
    /**
     * 加密一段任意長度的資料(ECB 區塊加密模式)
     * 請參閱："AES 任意長度資料加解密參數共同說明"
     * Note :
     *     本函式使用 ECB 區塊加密模式，速度較快，但使用在較大資料的加密時會透露出資料的某些特性，
     *     因此建議使用在小型資料的加密上(有關這個缺點的特性及個別個案的適用性請參考 ECB 區塊加密模式的敘述)。
     */
    aes_state_t  rndkeys[15*sizeof(aes_state_t)];
    size_t       rndkeys_cnt;
    size_t       outsz = get_encipher_buffer_size(insz);
    size_t       off;

    if( !in    ) return 0;
    if( !out   ) return outsz;
    if( !outsz ) return 0;

    // Calculate round keys
    rndkeys_cnt = aes_key_expand(rndkeys, key, keysz);
    if( !rndkeys_cnt ) return 0;
    // Copy data to the buffer
    memcpy(out, in, insz);
    if( outsz != insz ) memset(out+insz, 0, outsz-insz);
    // Encipher blocks
    for(off=0; off<outsz; off+=sizeof(aes_state_t))
        aes_state_encipher((aes_state_t*)(out+off), rndkeys, rndkeys_cnt);
    // Erase temporary data
    memset(rndkeys, 0, sizeof(rndkeys));

    return outsz;
}
//------------------------------------------------------------------------------
size_t aes_ecb_decipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz)
{
    /**
     * 解密一段任意長度的資料(ECB 區塊加密模式)
     * 請參閱："AES 任意長度資料加解密參數共同說明"
     * Note :
     *     本函式使用 ECB 區塊加密模式，速度較快，但使用在較大資料的加密時會透露出資料的某些特性，
     *     因此建議使用在小型資料的加密上(有關這個缺點的特性及個別個案的適用性請參考 ECB 區塊加密模式的敘述)。
     */
    aes_state_t rndkeys[15*sizeof(aes_state_t)];
    size_t      rndkeys_cnt;
    size_t      outsz = get_decipher_buffer_size(insz);
    size_t      off;

    if( !in    ) return 0;
    if( !out   ) return outsz;
    if( !outsz ) return 0;

    // Calculate round keys
    rndkeys_cnt = aes_key_expand(rndkeys, key, keysz);
    if( !rndkeys_cnt ) return 0;
    // Copy data to the buffer
    memcpy(out, in, insz);
    if( outsz != insz ) memset(out+insz, 0, outsz-insz);
    // Decipher blocks
    for(off=0; off<outsz; off+=sizeof(aes_state_t))
        aes_state_decipher((aes_state_t*)(out+off), rndkeys, rndkeys_cnt);
    // Erase temporary data
    memset(rndkeys, 0, sizeof(rndkeys));

    return outsz;
}
//------------------------------------------------------------------------------
void aes_cbc_get_initvec(void       /*out*/ *iv,
                         const void /*in*/  *data,
                         size_t     /*in*/   size)
{
    /**
     * 由傳入的未加密資料以平均取樣方式產生一個初始化向量(Initialisation Vector)
     * @iv   : 傳回 16 位元組的初始化向量。
     * @data : 用來取樣的資料，即為加密前的欲加密資料。
     * @size : 取樣資料的大小。
     * 取樣方式說明：
     * 1. 先將原始資料調整大小為 AES 區塊大小的整數倍(無條件進位並末尾補零)。
     * 2. 以 AES 區塊大小為資料分組格依據將資料進行分組。
     * 3. 設分組區塊數量為 N1，若 N1 < 3 則將 IV 內容全部填寫為零，並略過後面程序。
     * 4. 捨棄(完整移除)資料的首區塊及尾區塊，此時分組數量為 N2 = N1 - 2。
     * 5. IV 的第一個位元組即為所剩下資料的第一個位元組，之後每隔 N2 位元組取樣一次直到填滿 IV。
     *    示例如：
     *    IV[0] = DataRest[0*N2];
     *    IV[1] = DataRest[1*N2];
     *    IV[2] = DataRest[2*N2];
     *    IV[3] = DataRest[3*N2];
     *    ......
     */
    const byte_t *src  = data;
    byte_t       *dest = iv;
    size_t        step = ( size + 16 - 1 ) >> 4;
    int           n;

    if( !dest ) return;

    if( !src || step < 3 )
    {
        memset(dest, 0, sizeof(aes_state_t));
        return;
    }
    step -= 2;

    src += sizeof(aes_state_t);
    for(n=sizeof(aes_state_t);
        n--;
        ++dest, src+=step)
    {
        *dest = *src;
    }
}
//------------------------------------------------------------------------------
size_t aes_cbc_encipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz,
                        const void /*in*//*opt*/  *_iv)
{
    /**
     * 加密一段任意長度的資料(CBC 區塊加密模式)
     * 請參閱："AES 任意長度資料加解密參數共同說明"
     */
    aes_state_t  rndkeys[15*sizeof(aes_state_t)];
    size_t       rndkeys_cnt;
    aes_state_t  iv;
    size_t       outsz   = get_encipher_buffer_size(insz);
    size_t       statcnt = outsz >> 4;
    aes_state_t *stat    = (aes_state_t*)out;
    size_t       i;

    if( !in      ) return 0;
    if( !out     ) return outsz;
    if( !statcnt ) return 0;

    // Calculate round keys
    rndkeys_cnt = aes_key_expand(rndkeys, key, keysz);
    if( !rndkeys_cnt ) return 0;
    // Select IV
    if( _iv )
        memcpy(&iv, _iv, sizeof(iv));
    else
        aes_cbc_get_initvec(&iv, in, insz);
    // Copy data to the buffer
    memset(stat+statcnt-1, 0, sizeof(aes_state_t));
    memcpy(stat, in, insz);
    // Encipher blocks
    state_add(&stat[0], &iv);
    aes_state_encipher(&stat[0], rndkeys, rndkeys_cnt);
    for(i=1; i<statcnt; ++i)
    {
        state_add(&stat[i], &stat[i-1]);
        aes_state_encipher(&stat[i], rndkeys, rndkeys_cnt);
    }
    // Erase temporary data
    memset(rndkeys, 0, sizeof(rndkeys));
    memset(&iv    , 0, sizeof(iv));

    return outsz;
}
//------------------------------------------------------------------------------
size_t aes_cbc_decipher(void       /*out*//*opt*/ *out,
                        const void /*in*/         *in,
                        size_t     /*in*/          insz,
                        const void /*in*/         *key,
                        size_t     /*in*/          keysz,
                        const void /*in*//*opt*/  *_iv)
{
    /**
     * 解密一段任意長度的資料(CBC 區塊加密模式)
     * 請參閱："AES 任意長度資料加解密參數共同說明"
     */
    aes_state_t  rndkeys[15*sizeof(aes_state_t)];
    size_t       rndkeys_cnt;
    aes_state_t  iv;
    size_t       outsz   = get_decipher_buffer_size(insz);
    size_t       statcnt = outsz >> 4;
    aes_state_t *stat    = (aes_state_t*)out;
    aes_state_t *datbak  = NULL;
    size_t       i;

    if( !in      ) return 0;
    if( !out     ) return outsz;
    if( !statcnt ) return 0;

    // Calculate round keys
    rndkeys_cnt = aes_key_expand(rndkeys, key, keysz);
    if( !rndkeys_cnt ) return 0;
    // Copy data to the buffer
    datbak = malloc(outsz);
    if( !datbak )
    {
        memset(rndkeys, 0, sizeof(rndkeys));
        return 0;
    }
    memcpy(datbak, in, outsz);
    memcpy(stat  , in, outsz);
    // Decipher blocks
    for(i=0; i<statcnt; ++i)
        aes_state_decipher(&stat[i], rndkeys, rndkeys_cnt);
    for(i=1; i<statcnt; ++i)  // Decipher blocks except the first block
        state_add(&stat[i], &datbak[i-1]);
    // Select IV
    if( _iv )
        memcpy(&iv, _iv, sizeof(iv));
    else
        aes_cbc_get_initvec(&iv, (byte_t*)stat, insz);
    // Finish the first block
    state_add(&stat[0], &iv);
    // Erase temporary data and release buffer
    memset(rndkeys, 0, sizeof(rndkeys));
    memset(&iv    , 0, sizeof(iv));
    memset(datbak , 0, statcnt*sizeof(aes_state_t));
    free(datbak);

    return outsz;
}
//------------------------------------------------------------------------------

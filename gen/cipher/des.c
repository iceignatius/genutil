#include <assert.h>
#include <string.h>
#include "./des_bit.h"
#include "./des_table.h"
#include "./des.h"

//------------------------------------------------------------------------------
typedef union des_key_permuted_t
{
    sbit_t bits[56];

    struct des_key_permuted_part_t
    {
        sbit_t c[28];
        sbit_t d[28];
    } part;

} des_key_permuted_t;
//------------------------------------------------------------------------------
static
void mainkey_separate(sbit_t bits[64], const byte_t key[8])
{
    unsigned i;
    for(i=0; i<8; ++i)
        sbit_separate(&bits[i<<3], key[i]);
}
//------------------------------------------------------------------------------
static
void subkey_separate(sbit_t bits[48], const des_subkey_t *subkey)
{
    unsigned i;
    for(i=0; i<6; ++i)
        sbit_separate(&bits[i<<3], subkey->data[i]);
}
//------------------------------------------------------------------------------
static
void subkey_combine(des_subkey_t *subkey, const sbit_t bits[48])
{
    unsigned i;
    for(i=0; i<6; ++i)
        subkey->data[i] = sbit_combine(&bits[i<<3]);
}
//------------------------------------------------------------------------------
void des_key_expand(des_subkey_t /*out*/ subkeys[16], const byte_t /*in*/ mainkey[8])
{

    sbit_t             mkey_bits  [64];
    sbit_t             subkey_bits[48];
    des_key_permuted_t permuted;
    unsigned           i;

    mainkey_separate(mkey_bits, mainkey);

    sbit_permute(permuted.bits, 56, mkey_bits, des_table_pc_1);

    for(i=0; i<16; ++i)
    {
        sbit_shift_left_n(permuted.part.c, 28, des_table_shift_left[i]);
        sbit_shift_left_n(permuted.part.d, 28, des_table_shift_left[i]);

        sbit_permute(subkey_bits, 48, permuted.bits, des_table_pc_2);
        subkey_combine(&subkeys[i], subkey_bits);
    }

    // Erase key data
    memset(mkey_bits  , 0, sizeof(mkey_bits)  );
    memset(subkey_bits, 0, sizeof(subkey_bits));
    memset(&permuted  , 0, sizeof(permuted)   );
}
//------------------------------------------------------------------------------
void tdea_key_expand(byte_t /*out*/ keys[3*8], const byte_t /*in*/ *mainkey, size_t /*in*/ mainkey_size)
{
    if     ( mainkey_size >= 3*8 )
    {
        // The real Triple-DES
        memcpy(keys, mainkey, 3*8);
    }
    else if( mainkey_size >= 2*8 )
    {
        // Triple-DES double key mode
        memcpy( keys + 0*8, mainkey + 0*8, 8 );
        memcpy( keys + 1*8, mainkey + 1*8, 8 );
        memcpy( keys + 2*8, mainkey + 0*8, 8 );
    }
    else if( mainkey_size >= 1*8 )
    {
        // Back to the same as single DES
        memcpy( keys + 0*8, mainkey + 0*8, 8 );
        memcpy( keys + 1*8, mainkey + 0*8, 8 );
        memcpy( keys + 2*8, mainkey + 0*8, 8 );
    }
    else
    {
        // Undefined behavior
        memset(keys, 0, 3*8);
        memcpy( keys + 0*8, mainkey, mainkey_size );
        memcpy( keys + 1*8, mainkey, mainkey_size );
        memcpy( keys + 2*8, mainkey, mainkey_size );
    }
}
//------------------------------------------------------------------------------
//---- Private Cipher Routines -------------------------------------------------
//------------------------------------------------------------------------------
typedef union des_f48_t
{
    sbit_t bits[48];
    sbit_t s[8][6];
} des_f48_t;
typedef union des_f32_t
{
    sbit_t bits[32];
    sbit_t s[8][4];
} des_f32_t;
//------------------------------------------------------------------------------
static
void func_s(sbit_t dest[4], const sbit_t src[6], unsigned sind)
{
    unsigned row, col, val;

    row     = ( src[0] <<1 ) | src[5];
    col     = ( src[1] <<3 ) | ( src[2] << 2 ) | ( src[3] << 1 ) | src[4];
    val     = des_table_s[sind][row][col];
    dest[0] = ( val >> 3 ) & 0x01;
    dest[1] = ( val >> 2 ) & 0x01;
    dest[2] = ( val >> 1 ) & 0x01;
    dest[3] =   val        & 0x01;
}
//------------------------------------------------------------------------------
static
void func_f(sbit_t /*in,out*/ data[32], const sbit_t /*in*/ subkey[48])
{
    des_f32_t unit32;
    des_f48_t unit48;
    unsigned  i;

    memcpy(unit32.bits, data, sizeof(unit32.bits));
    sbit_permute(unit48.bits, 48, unit32.bits, des_table_e);
    sbit_xor(unit48.bits, unit48.bits, subkey, 48);
    for(i=0; i<8; ++i)
        func_s(unit32.s[i], unit48.s[i], i);
    sbit_permute(data, 32, unit32.bits, des_table_p);
}
//------------------------------------------------------------------------------
#ifdef DES_DEBUG_TEST
static
void func_s_test(void)
{
    static const sbit_t bits_in1 [6] = { 0,1,1,0,0,0 };
    static const sbit_t bits_in2 [6] = { 0,1,0,0,0,1 };
    static const sbit_t bits_in3 [6] = { 0,1,1,1,1,0 };
    static const sbit_t bits_in4 [6] = { 1,1,1,0,1,0 };
    static const sbit_t bits_in5 [6] = { 1,0,0,0,0,1 };
    static const sbit_t bits_in6 [6] = { 1,0,0,1,1,0 };
    static const sbit_t bits_in7 [6] = { 0,1,0,1,0,0 };
    static const sbit_t bits_in8 [6] = { 1,0,0,1,1,1 };
    static const sbit_t bits_out1[4] = { 0,1,0,1 };
    static const sbit_t bits_out2[4] = { 1,1,0,0 };
    static const sbit_t bits_out3[4] = { 1,0,0,0 };
    static const sbit_t bits_out4[4] = { 0,0,1,0 };
    static const sbit_t bits_out5[4] = { 1,0,1,1 };
    static const sbit_t bits_out6[4] = { 0,1,0,1 };
    static const sbit_t bits_out7[4] = { 1,0,0,1 };
    static const sbit_t bits_out8[4] = { 0,1,1,1 };

    sbit_t bits[4];

    func_s(bits, bits_in1, 1-1);  assert( 0 == memcmp(bits, bits_out1, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in2, 2-1);  assert( 0 == memcmp(bits, bits_out2, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in3, 3-1);  assert( 0 == memcmp(bits, bits_out3, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in4, 4-1);  assert( 0 == memcmp(bits, bits_out4, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in5, 5-1);  assert( 0 == memcmp(bits, bits_out5, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in6, 6-1);  assert( 0 == memcmp(bits, bits_out6, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in7, 7-1);  assert( 0 == memcmp(bits, bits_out7, 4*sizeof(sbit_t)) );
    func_s(bits, bits_in8, 8-1);  assert( 0 == memcmp(bits, bits_out8, 4*sizeof(sbit_t)) );
}
//------------------------------------------------------------------------------
static
void func_f_test(void)
{
    static const sbit_t k1[48] =
    {
        0,0,0,1,1,0,
        1,1,0,0,0,0,
        0,0,1,0,1,1,
        1,0,1,1,1,1,
        1,1,1,1,1,1,
        0,0,0,1,1,1,
        0,0,0,0,0,1,
        1,1,0,0,1,0,
    };

    static const sbit_t data_in[32] =
    {
        1,1,1,1,0,0,0,0,
        1,0,1,0,1,0,1,0,
        1,1,1,1,0,0,0,0,
        1,0,1,0,1,0,1,0
    };

    static const sbit_t data_out[32] =
    {
        0,0,1,0,0,0,1,1,
        0,1,0,0,1,0,1,0,
        1,0,1,0,1,0,0,1,
        1,0,1,1,1,0,1,1
    };

    sbit_t data[32];

    memcpy(data, data_in, sizeof(data_in));
    func_f(data, k1);
    assert( 0 == memcmp(data, data_out, sizeof(data_out)) );
}
//------------------------------------------------------------------------------
void des_private_debug_test(void)
{
    func_s_test();
    func_f_test();
}
#endif
//------------------------------------------------------------------------------
//---- Block Unit Cipher Routine -----------------------------------------------
//------------------------------------------------------------------------------
typedef union des_permuted_t
{
    sbit_t bits[64];

    struct des_permuted_part_t
    {
        sbit_t l[32];
        sbit_t r[32];
    } part;

} des_permuted_t;
//------------------------------------------------------------------------------
static
void cipher_step(des_permuted_t /*in,out*/ *block, const sbit_t /*in*/ subkey[48])
{
    sbit_t temp[32];

    // Backup part R
    memcpy(temp, block->part.r, 32*sizeof(sbit_t));

    // Calculate the new part R
    func_f  (block->part.r, subkey);
    sbit_xor(block->part.r, block->part.r, block->part.l, 32);

    // The new part L is the old part R
    memcpy(block->part.l, temp, 32*sizeof(sbit_t));
}
//------------------------------------------------------------------------------
static
void swap_block_parts(des_permuted_t /*in,out*/ *block)
{
    sbit_t temp[32];

    memcpy(temp         , block->part.l, 32*sizeof(sbit_t));
    memcpy(block->part.l, block->part.r, 32*sizeof(sbit_t));
    memcpy(block->part.r, temp         , 32*sizeof(sbit_t));
}
//------------------------------------------------------------------------------
void des_block_cipher(byte_t /*in,out*/ block[8], const byte_t /*in*/ key[8], bool /*in*/ reverse_subkeys)
{
    des_subkey_t   subkeys    [16];
    sbit_t         subkey_bits[48];
    sbit_t         block_bits [64];
    des_permuted_t permuted;
    unsigned       i;

    des_key_expand(subkeys, key);

    for(i=0; i<8; ++i) sbit_separate(&block_bits[i<<3], block[i]);

    sbit_permute(permuted.bits, 64, block_bits, des_table_ip);

    for(i=0; i<16; ++i)
    {
        unsigned keyindex = ( reverse_subkeys )?( 16 - 1 - i ):( i );

        subkey_separate(subkey_bits, &subkeys[keyindex]);
        cipher_step(&permuted, subkey_bits);
    }
    swap_block_parts(&permuted);

    sbit_permute(block_bits, 64, permuted.bits, des_table_iip);

    for(i=0; i<8; ++i) block[i] = sbit_combine(&block_bits[i<<3]);

    // Erase key data
    memset(subkeys    , 0, sizeof(subkeys)    );
    memset(subkey_bits, 0, sizeof(subkey_bits));
}
//------------------------------------------------------------------------------
//---- Multi Block Cipher ------------------------------------------------------
//------------------------------------------------------------------------------
typedef struct des_block_t
{
    byte_t data[8];
} des_block_t;
//------------------------------------------------------------------------------
static
size_t get_encipher_buffer_size(size_t datasize)
{
    return ( datasize + 8 - 1 ) & ~0x07;
}
//------------------------------------------------------------------------------
static
size_t get_decipher_buffer_size(size_t datasize)
{
    return datasize & ~0x07;
}
//------------------------------------------------------------------------------
size_t tdea_ecb_encipher(void       /*out*//*opt*/ *out,
                         const void /*in*/         *in,
                         size_t     /*in*/          insz,
                         const void /*in*/         *key,
                         size_t     /*in*/          keysz)
{
    size_t       outsz    = get_encipher_buffer_size(insz);
    des_block_t *block    = out;
    size_t       blockcnt = outsz/sizeof(des_block_t);
    byte_t       tdeakey[24];

    assert( outsz >= insz );
    if( !in  || !insz  ) return 0;
    if( !key || !keysz ) return 0;
    if( !block    ) return outsz;
    if( !blockcnt ) return 0;

    memcpy(out, in, insz);
    memset(out+insz, 0, outsz-insz);

    tdea_key_expand(tdeakey, key, keysz);
    while( blockcnt )
    {
        tdea_block_encipher(block->data, tdeakey);

        ++block;
        --blockcnt;
    }

    // Erase key data
    memset(tdeakey, 0, sizeof(tdeakey));

    return outsz;
}
//------------------------------------------------------------------------------
size_t tdea_ecb_decipher(void       /*out*//*opt*/ *out,
                         const void /*in*/         *in,
                         size_t     /*in*/          insz,
                         const void /*in*/         *key,
                         size_t     /*in*/          keysz)
{
    size_t       outsz    = get_decipher_buffer_size(insz);
    des_block_t *block    = out;
    size_t       blockcnt = outsz/sizeof(des_block_t);
    byte_t       tdeakey[24];

    assert( outsz <= insz );
    if( !in  || !insz  ) return 0;
    if( !key || !keysz ) return 0;
    if( !block    ) return outsz;
    if( !blockcnt ) return 0;

    memcpy(out, in, outsz);

    tdea_key_expand(tdeakey, key, keysz);
    while( blockcnt )
    {
        tdea_block_decipher(block->data, tdeakey);

        ++block;
        --blockcnt;
    }

    // Erase key data
    memset(tdeakey, 0, sizeof(tdeakey));

    return outsz;
}
//------------------------------------------------------------------------------
void tdea_cbc_get_initvec(byte_t      /*out*/  iv[8],
                          const void  /*in*/  *data,
                          size_t      /*in*/   size)
{
    /**
     * 由傳入的未加密資料以平均取樣方式產生一個初始化向量(Initialisation Vector)
     * @iv   : 傳回函式所產生的初始化向量。
     * @data : 用來取樣的資料，即為加密前的欲加密資料。
     * @size : 取樣資料的大小。
     * 取樣方式說明：
     * 1. 先將原始資料調整大小為 DES 區塊大小的整數倍(無條件進位並末尾補零)。
     * 2. 以 DES 區塊大小為資料分組格依據將資料進行分組。
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
    static const unsigned blocksize = 8;
    const byte_t *src  = data;
    byte_t       *dest = (byte_t*) iv;
    size_t        step = ( size + 8 - 1 ) >> 3;
    int           n;

    if( !dest ) return;

    if( !src || step < 3 )
    {
        memset(dest, 0, blocksize);
        return;
    }
    step -= 2;

    src += blocksize;
    for(n=blocksize;
        n--;
        ++dest, src+=step)
    {
        *dest = *src;
    }
}
//------------------------------------------------------------------------------
static
void des_block_xor(des_block_t *dest, const des_block_t *src1, const des_block_t *src2)
{
    assert( dest && src1 && src2 );

    int i;
    for(i=0; i<sizeof(des_block_t); ++i)
        dest->data[i] = src1->data[i] ^ src2->data[i];
}
//------------------------------------------------------------------------------
size_t tdea_cbc_encipher(void         /*out*//*opt*/ *out,
                         const void   /*in*/         *in,
                         size_t       /*in*/          insz,
                         const void   /*in*/         *key,
                         size_t       /*in*/          keysz,
                         const byte_t /*in*//*opt*/   _iv[8])
{
    byte_t       tdeakey[24];
    des_block_t  iv;
    size_t       outsz    = get_encipher_buffer_size(insz);
    size_t       blockcnt = outsz >> 3;
    des_block_t *block    = (des_block_t*)out;
    size_t       i;

    if( !in       ) return 0;
    if( !out      ) return outsz;
    if( !blockcnt ) return 0;

    // Expand key
    tdea_key_expand(tdeakey, key, keysz);
    // Select IV
    if( _iv ) memcpy(&iv, _iv, sizeof(des_block_t));
    else      tdea_cbc_get_initvec(iv.data, in, insz);
    // Copy data to out buffer
    memcpy(out, in, insz);
    memset(out+insz, 0, outsz-insz);
    // Encipher block by block
    des_block_xor(&block[0], &block[0], &iv);
    tdea_block_encipher(block[0].data, key);
    for(i=1; i<blockcnt; ++i)
    {
        des_block_xor(&block[i], &block[i], &block[i-1]);
        tdea_block_encipher(block[i].data, tdeakey);
    }
    // Erase key data
    memset(tdeakey, 0, sizeof(tdeakey));

    return outsz;
}
//------------------------------------------------------------------------------
size_t tdea_cbc_decipher(void         /*out*//*opt*/ *out,
                         const void   /*in*/         *in,
                         size_t       /*in*/          insz,
                         const void   /*in*/         *key,
                         size_t       /*in*/          keysz,
                         const byte_t /*in*//*opt*/   _iv[8])
{
    byte_t       tdeakey[24];
    des_block_t  iv;
    size_t       outsz    = get_decipher_buffer_size(insz);
    size_t       blockcnt = outsz >> 3;
    des_block_t *block    = (des_block_t*)out;
    size_t       i;

    if( !in       ) return 0;
    if( !out      ) return outsz;
    if( !blockcnt ) return 0;

    // Expand key
    tdea_key_expand(tdeakey, key, keysz);
    // Copy data to out buffer
    memcpy(out, in, outsz);
    // Decipher block by block, exclude the first block.
    for(i=blockcnt-1; i>0; --i)
    {
        tdea_block_decipher(block[i].data, tdeakey);
        des_block_xor(&block[i], &block[i], &block[i-1]);
    }
    // Select IV
    if( _iv ) memcpy(&iv, _iv, sizeof(des_block_t));
    else      tdea_cbc_get_initvec(iv.data, out, outsz);
    // Decipher the first block
    tdea_block_decipher(block[0].data, tdeakey);
    des_block_xor(&block[0], &block[0], &iv);
    // Erase key data
    memset(tdeakey, 0, sizeof(tdeakey));

    return outsz;
}
//------------------------------------------------------------------------------

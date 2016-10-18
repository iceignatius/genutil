#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../minmax.h"
#include "../bignum/const.h"
#include "../bignum/io.h"
#include "../bignum/math.h"
#include "rsa.h"

#if BIGNUM_DIGSZ < 2*RSA_KEY_PARAMSZ
#error BIGNUM_DIGSZ must be great equal to 2*RSA_KEY_PARAMSZ
#endif

//------------------------------------------------------------------------------
static
uintbig_t make_rand_prime(unsigned int ndig, mt_rand64_t *rndobj)
{
    static const unsigned int testtimes = 8;
    uintbig_t ret;

    do
    {
        ndig = MAX( 1 , MIN( ndig , BIGNUM_DIGSZ ) );
        ret  = uintbig_rand_ndig(ndig, rndobj);
        ret.digit[     0] |= 0x01;
        ret.digit[ndig-1] |= 0xC0;
    } while( !uintbig_prime_test(&ret, testtimes, rndobj) );

    return ret;
}
//------------------------------------------------------------------------------
static
bool rsa_test_keys(const uintbig_t *n,
                   const uintbig_t *e,
                   const uintbig_t *d,
                   mt_rand64_t     *rndobj)
{
    uintbig_t dat0, dat1;
    bool      res;

    assert( n && e && d );
    assert( n->ndig > 1 );

    dat0 = uintbig_rand_ndig(n->ndig-1 ,rndobj);
    dat1 = uintbig_powermod(&dat0, e, n);
    dat1 = uintbig_powermod(&dat1, d, n);
    res  = uintbig_is_equal(&dat0, &dat1);

    memset(&dat0, 0, sizeof(uintbig_t));
    memset(&dat1, 0, sizeof(uintbig_t));

    return res;
}
//------------------------------------------------------------------------------
void rsa_generate_keys(rsa_key_t   /*out*/         *publickey,
                       rsa_key_t   /*out*/         *privatekey,
                       rsa_type_t  /*in */          type,
                       mt_rand64_t /*in,optional*/ *rndobj)
{
    /**
     * Generate a pare of random keys
     */
    unsigned int rndndig;
    uintbig_t    n, m, e, d;
    uintbig_t   *p = &e;
    uintbig_t   *q = &d;
    uintbig_t    e_min;
    uintbig_t    em_gcd;

    if( !publickey || !privatekey ) return;

    // Select parameter digits count
    rndndig = rsa_get_state_size(type) >> 1;
    if( !rndndig ) return;

    do
    {
        // Select prime number P and Q
        *p = make_rand_prime(rndndig+1, rndobj);
        *q = make_rand_prime(rndndig-1, rndobj);

        // Calculate N=P*Q and M=(P-1)*(Q-1)
        n = uintbig_mul(p, q);
        uintbig_minusminus(p);
        uintbig_minusminus(q);
        m = uintbig_mul(p, q);

        // Select E, and GCS(E,M)=1, E < M.
        // Calculate D, and D*E mod M=1.
        e_min = m;
        uintbig_byteshift_right(&e_min, 1);
        do
        {
            e      = uintbig_rand_range(&e_min, &m, rndobj);
            em_gcd = uintbig_gcd(&e, &m);
        } while( !uintbig_is_equal(&em_gcd, &uintbig_const_one) ||
                 !uintbig_modmulinv(&e, &m, &d)                 );
    } while( !rsa_test_keys(&n, &e, &d, rndobj) );

    // The public key is (N,E), and the private key is (N,D)
    memset(publickey , 0, sizeof(rsa_key_t));
    memset(privatekey, 0, sizeof(rsa_key_t));
    memcpy(publickey ->modulus , n.digit, n.ndig);
    memcpy(publickey ->exponent, e.digit, e.ndig);
    memcpy(privatekey->modulus , n.digit, n.ndig);
    memcpy(privatekey->exponent, d.digit, d.ndig);

    // Erase data
    memset(&n     , 0, sizeof(uintbig_t));
    memset(&m     , 0, sizeof(uintbig_t));
    memset(&e     , 0, sizeof(uintbig_t));
    memset(&d     , 0, sizeof(uintbig_t));
    memset(&e_min , 0, sizeof(uintbig_t));
    memset(&em_gcd, 0, sizeof(uintbig_t));
}
//------------------------------------------------------------------------------
size_t rsa_get_state_size(rsa_type_t type)
{
    /**
     * 查詢不同位元型態的 RSA 加解密
     */
    switch( type )
    {
    case rsa_type_128  :  return  16;
    case rsa_type_256  :  return  32;
    case rsa_type_512  :  return  64;
    case rsa_type_768  :  return  96;
    case rsa_type_1024 :  return 128;
    default            :  return   0;
    }
}
//------------------------------------------------------------------------------
size_t rsa_state_encipher(void            /*out*/ *out,
                          size_t          /*in */  outsz,
                          const void      /*in */ *in,
                          size_t          /*in */  insz,
                          const rsa_key_t /*in */ *key)
{
    /**
     * 加密區塊資料
     * @out    : 用來接收加解密後資料的緩衝區，此緩衝區需已配置足夠的大小；
     *           若此輸入為 NULL，則函式將傳回此緩衝區所需的最小大小。
     * @outsz  : 資料緩衝區的大小。
     * @in     : 加解密前資料輸入。
     * @insz   : 加解密前資料大小。注意資料中超出加解密區塊大小的資料將被忽
     *           略，而不足的部份將被從尾端補零。不同位元型態的 RSA 加解密機制
     *           有不同的區塊大小，若使用者對 RSA 加解密機制沒有研究的話，可以
     *           透過 rsa_get_state_size 函式查詢不同位元型態加解密機制的區塊大
     *           小，欲加密的資料大小需小於這個數值。
     * @key    : 加解祕密鑰輸入。
     * @return : 函式操作成功時回傳寫入輸出緩衝區的資料大小；
     *           當 out 為 NULL 時傳回 out 所需的緩衝區大小；
     *           其他失敗情況傳回零。
     */
    uintbig_t n, e;
    uintbig_t text, code;

    if( !key ) return 0;
    n = uintbig_from_bin(key->modulus , RSA_KEY_PARAMSZ);
    e = uintbig_from_bin(key->exponent, RSA_KEY_PARAMSZ);

    if( !out ) return n.ndig;
    if( outsz < (size_t)n.ndig || !in || !insz ) return 0;

    text = uintbig_from_bin(in, insz);
    code = uintbig_powermod(&text, &e, &n);
    memcpy(out, code.digit, n.ndig);

    memset(&e   , 0, sizeof(uintbig_t));
    memset(&text, 0, sizeof(uintbig_t));
    memset(&code, 0, sizeof(uintbig_t));

    return n.ndig;
}
//------------------------------------------------------------------------------
size_t rsa_state_decipher(void            /*out*/ *out,
                          size_t          /*in */  outsz,
                          const void      /*in */ *in,
                          size_t          /*in */  insz,
                          const rsa_key_t /*in */ *key)
{
    /**
     * 解密區塊資料
     * @out    : 用來接收加解密後資料的緩衝區，此緩衝區需已配置足夠的大小；
     *           若此輸入為 NULL，則函式將傳回此緩衝區可能需要的最大大小。
     * @outsz  : 資料緩衝區的大小，若緩衝區大小小於加密區塊大小，則超出緩衝
     *           區大小的資料將被忽略。
     * @in     : 加解密前資料輸入。
     * @insz   : 加解密前資料大小。注意資料中超出加解密區塊大小的資料將被忽
     *           略，而不足的部份將被從尾端補零。不同位元型態的 RSA 加解密機
     *           制有不同的區塊大小，若使用者對 RSA 加解密機制沒有研究的話，
     *           可以透過 rsa_get_state_size 函式查詢不同位元型態加解密機制的
     *           區塊大小，建議解密前資料大小應等同這個數值。
     * @key    : 加解祕密鑰輸入。
     * @return : 函式操作成功時回傳寫入輸出緩衝區的資料大小；
     *           當 out 為 NULL 時傳回 out 可能需要的最大大小；
     *           其他失敗情況傳回零。
     */
    uintbig_t n, e;
    uintbig_t code, text;

    if( !key ) return 0;
    n = uintbig_from_bin(key->modulus , RSA_KEY_PARAMSZ);
    e = uintbig_from_bin(key->exponent, RSA_KEY_PARAMSZ);

    if( !out ) return n.ndig;
    if( !in || !insz ) return 0;

    code  = uintbig_from_bin(in, insz);
    text  = uintbig_powermod(&code, &e, &n);
    outsz = MIN( outsz, (size_t)n.ndig );
    memcpy(out, text.digit, outsz);

    memset(&e   , 0, sizeof(uintbig_t));
    memset(&code, 0, sizeof(uintbig_t));
    memset(&text, 0, sizeof(uintbig_t));

    return outsz;
}
//------------------------------------------------------------------------------

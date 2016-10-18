#include <assert.h>
#include <stddef.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../minmax.h"
#include "./const.h"
#include "./math.h"

//------------------------------------------------------------------------------
//---- Basic Operator ----------------------------------------------------------
//------------------------------------------------------------------------------
uintbig_t uintbig_add(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * a add b
     */
    int       carry = 0;
    int       i, ndigmax;
    uintbig_t ret;

    assert( a && b );

    if( a->ndig <= 0 || b ->ndig <= 0 ) return uintbig_const_nan;

    ndigmax = MIN( MAX( a->ndig , b->ndig ) + 1 , BIGNUM_DIGSZ );
    for(i=0; i<ndigmax; ++i)
    {
        int temp;

        temp         = a->digit[i] + b->digit[i] + carry;
        ret.digit[i] = temp;
        carry        = temp >> 8;
    }
    while( i<BIGNUM_DIGSZ )
    {
        ret.digit[i++] = 0;
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_sub(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * a subtract b
     */
    int       borrow = 0;
    int       i, ndigmax;
    uintbig_t ret;

    assert( a && b );

    if( a->ndig <= 0 || b ->ndig <= 0 ) return uintbig_const_nan;

    ndigmax = MAX( a->ndig , b->ndig );
    for(i=0; i<ndigmax; ++i)
    {
        int temp;

        temp = (int)a->digit[i] - (int)b->digit[i] - borrow;
        if( temp < 0 )
        {
            ret.digit[i] = temp + 0x100;
            borrow       = 1;
        }
        else
        {
            ret.digit[i] = temp;
            borrow       = 0;
        }
    }

    if( borrow )
    {
        while( i < BIGNUM_DIGSZ )
            ret.digit[i++] = 0xFF;
    }
    else
    {
        while( i < BIGNUM_DIGSZ )
            ret.digit[i++] = 0;
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
static
uintbig_t uintbig_mulint(const uintbig_t *a, int b)
{
    int            carry = 0;
    int            ndig, nzeros;
    const uint8_t *src;
    uint8_t       *dest;
    uintbig_t      ret;

    assert( a && a->ndig > 0 );

    ndig   = MIN( a->ndig + 1 , BIGNUM_DIGSZ );
    nzeros = BIGNUM_DIGSZ - ndig;
    src    = a->digit;
    dest   = ret.digit;

    while( ndig-- )
    {
        int temp;

        temp    = *src++ * b + carry;
        *dest++ = temp  & 0xFF;
        carry   = temp >> 8;
    }
    while( nzeros-- )
    {
        *dest++ = 0;
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
static
uintbig_t uintbig_mulint_and_byteshift_left(const uintbig_t *a, int b, unsigned int s)
{
    int            carry = 0;
    int            ndig, nzeros;
    const uint8_t *src;
    uint8_t       *dest;
    uintbig_t      ret;

    assert( a && a->ndig > 0 );

    ndig   = MIN( a->ndig + 1 , BIGNUM_DIGSZ - (int)s );
    nzeros = BIGNUM_DIGSZ - ndig - s;
    src    = a->digit;
    dest   = ret.digit;

    while( s-- )
    {
        *dest++ = 0;
    }
    while( ndig-- )
    {
        int temp;

        temp    = *src++ * b + carry;
        *dest++ = temp  & 0xFF;
        carry   = temp >> 8;
    }
    while( nzeros-- )
    {
        *dest++ = 0;
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_mul(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * a multiply b
     */
    int       i;
    uintbig_t ret;

    assert( a && b );

    if( a->ndig <= 0 || b ->ndig <= 0 ) return uintbig_const_nan;

    ret = uintbig_mulint(a, b->digit[0]);
    for(i=1; i<b->ndig; ++i)
    {
        uintbig_t temp = uintbig_mulint_and_byteshift_left(a, b->digit[i], i);
        ret = uintbig_add(&ret, &temp);
    }

    return ret;
}
//------------------------------------------------------------------------------
static
int divmod_directly(const uintbig_t *a, const uintbig_t *b, uintbig_t *rem)
{
    int q = 0;

    assert( a && b && rem );

    *rem = *a;
    while( uintbig_is_greatthen(rem, b) )
    {
        ++q;
        *rem = uintbig_sub(rem, b);
    }
    if( uintbig_is_equal(rem, b) )
    {
        ++q;
        *rem = uintbig_const_zero;
    }

    return q;
}
//------------------------------------------------------------------------------
static
int divmod_bisection(const uintbig_t *a, const uintbig_t *b, uintbig_t *rem)
{
    /**
     * Note and WARNING :
     *
     * The quotient range supports are between 0 to 255,
     * that is the input 'a' and 'b' must have a quotient in this range,
     * or the result will be incorrectly!
     */
    int       q, ql, qu;
    uintbig_t fq;

    assert( a && b && rem );

    // Special cases
    if( uintbig_is_zero(a) )
    {
        *rem = uintbig_const_zero;
        return 0;
    }
    if( uintbig_is_equal(a,b) )
    {
        *rem = uintbig_const_zero;
        return 1;
    }
    if( uintbig_is_lessthen(a,b) )
    {
        *rem = *a;
        return 0;
    }

    // Get initial value
    // - Lower
    ql = 1;
    // - Upper
    qu = 0xFF;
    fq = uintbig_mulint(b, qu);
    if( uintbig_is_lessequal(&fq, a) )
    {
        *rem = uintbig_sub(a, &fq);
        return qu;
    }
    // Solve quotient
    while(( qu - ql > 1 )||( q != ql ))
    {
        q  = ( ql + qu ) >> 1;
        fq = uintbig_mulint(b, q);
        if( uintbig_is_greatthen(&fq, a) )
        {
            qu = q;
        }
        else if( uintbig_is_lessthen(&fq, a) )
        {
            ql = q;
        }
        else
        {
            *rem = uintbig_const_zero;
            return q;
        }
    }
    // Solve remainder
    *rem = uintbig_sub(a, &fq);

    return q;
}
//------------------------------------------------------------------------------
void uintbig_divmod(const uintbig_t *a, const uintbig_t *b, uintbig_t *quo, uintbig_t *rem)
{
    /**
     * Calculate the quotient and remainder of 'a' by 'b'
     *
     * WARNING : The memory pointer value of 'quo' or 'rem' CANNOT be the same value of 'a' or 'b'!
     *
     * Note of special cases :
     *
     * 1. if 'a' is NAN or 'b' is NAN, then
     *    quo will be NAN, and
     *    rem will be ZERO.
     *
     * 2. if 'b' is ZERO, then
     *    quo will be NAN, and
     *    rem will be ZERO.
     *
     * 3. if 'a' is ZERO, then
     *    quo will be ZERO, and
     *    rem will be ZERO.
     *
     * 4. if 'a' is equal to 'b', then
     *    quo will be ONE, and
     *    rem will be ZERO.
     *
     * 5. if 'a' less then 'b', then
     *    quo will be ZERO, and
     *    rem will be the same of 'a'.
     */
    int i, nquo;

    assert( a && b && quo && rem );

    // Special cases
    if( a->ndig <= 0 || b->ndig <= 0 )
    {
        *quo = uintbig_const_nan;
        *rem = uintbig_const_zero;
        return;
    }
    if( uintbig_is_zero(b) )
    {
        *quo = uintbig_const_nan;
        *rem = uintbig_const_zero;
        return;
    }
    if( uintbig_is_zero(a) )
    {
        *quo = uintbig_const_zero;
        *rem = uintbig_const_zero;
        return;
    }
    if( uintbig_is_equal(a,b) )
    {
        *quo = uintbig_const_one;
        *rem = uintbig_const_zero;
        return;
    }
    if( uintbig_is_lessthen(a,b) )
    {
        *quo = uintbig_const_zero;
        *rem = *a;
        return;
    }

    // Solve quotient and remainder
    if( b->ndig == BIGNUM_DIGSZ )
    {
        // Solve by clumsy iteration if the value of 'b' is VERY large.
        uintbig_set_dig(quo, divmod_directly(a, b, rem));
    }
    else
    {
        // Solve by long division
        nquo = a->ndig - b->ndig + 1;
        *rem = *a;
        uintbig_byteshift_right(rem, nquo );
        for(i=BIGNUM_DIGSZ-1; i>=nquo; --i)
        {
            quo->digit[i] = 0;
        }
        for(i=nquo-1; i>=0; --i)
        {
            uintbig_byteshift_left(rem, 1);
            rem->digit[0] = a->digit[i];
            quo->digit[i] = divmod_bisection(rem, b, rem);
        }
        // Update digit usage count
        quo->ndig = uintbig_get_digit_usage(quo->digit);
    }
}
//------------------------------------------------------------------------------
uintbig_t uintbig_div(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * a divide b
     */
    uintbig_t quo, rem;

    assert( a && b );

    uintbig_divmod(a, b, &quo, &rem);
    return quo;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_mod(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * a modulo b
     */
    uintbig_t quo, rem;

    assert( a && b );

    uintbig_divmod(a, b, &quo, &rem);
    return rem;
}
//------------------------------------------------------------------------------
void uintbig_plusplus(uintbig_t *a)
{
    /**
     * ++a
     */
    bool carry;

    assert( a );

    if( a->ndig <= 0 ) return;

    carry = !( ++(a->digit[0]) & 0xFF );
    if( carry )
    {
        int i, ndigmax;

        ndigmax = MIN( a->ndig + 1 , BIGNUM_DIGSZ );
        for(( i=1 );( carry && i < ndigmax );( ++i ))
            carry = !( ++(a->digit[i]) & 0xFF );

        if( i > a->ndig ) a->ndig = i;
    }
}
//------------------------------------------------------------------------------
void uintbig_minusminus(uintbig_t *a)
{
    /**
     * --a
     */
    bool borrow;
    int  i;

    assert( a );

    if( a->ndig <= 0 ) return;

    borrow = !( a->digit[0]-- );
    if( borrow )
    {
        for(( i=1 );( borrow && i<BIGNUM_DIGSZ );( ++i ))
            borrow = !( a->digit[i]-- );

        if( i > a->ndig )
            a->ndig = i;
        else if(( i > 1 )&&( i == a->ndig )&&( !a->digit[a->ndig-1] ))
            --a->ndig;
    }
}
//------------------------------------------------------------------------------
//---- Advanced Functions ------------------------------------------------------
//------------------------------------------------------------------------------
uintbig_t uintbig_power2(const uintbig_t *a)
{
    /**
     * Calculate 'a' to the power of 2
     *
     *
     * 演算法分析筆記：
     *
     * Note :
     * 1. The unit of data shift is BYTE
     * 2. a^n means 'a' to the power of 'n', not XOR!
     *
     * [10]  [9]  [8]  [7]  [6]  [5]  [4]  [3]  [2]  [1]  [0]
     *                            a5   a4   a3   a2   a1   a0
     * X                          a5   a4   a3   a2   a1   a0
     * ------------------------------------------------------
     *                          a0a5 a0a4 a0a3 a0a2 a0a1 A0A0
     *                     a1a5 a1a4 a1a3 a1a2 A1A1 a0a1
     *                a2a5 a2a4 a2a3 A2A2 a1a2 a0a2
     *           a3a5 a3a4 A3A3 a2a3 a1a3 a0a3
     *      a4a5 A4A4 a3a4 a2a4 a1a4 a0a4
     * A5A5 a4a5 a3a5 a2a5 a1a5 a0a5
     *
     * (a0^2)<<0 + (a1^2)<<2 + (a2^2)<<4 + (a3^2)<<6 + (a4^2)<<8 + (a5^2)<<10
     *  + (2*a0*a1)<<1 + (2*a0*a2)<<2 + (2*a0*a3)<<3 + (2*a0*a4)<<4 + (2*a0*a5)<<5
     *  + (2*a1*a2)<<3 + (2*a1*a3)<<4 + (2*a1*a4)<<5 + (2*a1*a5)<<6
     *  + (2*a2*a3)<<5 + (2*a2*a4)<<6 + (2*a2*a5)<<7
     *  + (2*a3*a4)<<7 + (2*a3*a5)<<8
     *  + (2*a4*a5)<<9
     */
#ifndef UNDEF
    // 因為 power2 的演算法存在運算效率低落的問題，因此在改進之前將其封存，
    // 改為呼叫乘法函式。
    return uintbig_mul(a,a);
#else
    uintbig_t ret;
    int       i, k, off0;

    assert( a );

    if( a->ndig <= 0       ) return uintbig_const_nan;
    if( uintbig_is_zero(a) ) return uintbig_const_zero;

    ret = uintbig_const_zero;
    for( i=0 ; i < MIN( a->ndig , BIGNUM_DIGSZ/2 ) ; ++i )
    {
        int off = i << 1;
        int val = a->digit[i] * a->digit[i];

        ret.digit[off++] = val & 0xFF;
        if( off != BIGNUM_DIGSZ )
            ret.digit[off] = val >> 8;
    }
    ret.ndig = uintbig_get_digit_usage(ret.digit);

    for(( k=0, off0=1 );(( k < a->ndig )&&( off0 < BIGNUM_DIGSZ ));( ++k, off0+=2 ))
    {
        int off;

        for(( i=k+1, off=off0 );(( i < a->ndig )&&( off < BIGNUM_DIGSZ ));( ++i, ++off ))
        {
            int       val;
            uintbig_t temp;

            val           = a->digit[k] * a->digit[i];
            temp          = uintbig_const_zero;
            temp.digit[0] = val & 0xFF;
            temp.digit[1] = val >> 8;
            temp.ndig     = temp.digit[1] ? 2 : 1;

            temp = uintbig_add(&temp, &temp);
            uintbig_byteshift_left(&temp, off);

            ret = uintbig_add(&ret, &temp);
        }
    }
    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
#endif
}
//------------------------------------------------------------------------------
uintbig_t uintbig_power(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * Calculate 'a' to the power of 'b'
     */
    uintbig_t ret;
    int       idig, ibit;

    assert( a && b );

    if( a->ndig <= 0 || b->ndig <= 0 ) return uintbig_const_nan;
    if( uintbig_is_zero(b)           ) return uintbig_const_one;
    if( uintbig_is_zero(a)           ) return uintbig_const_zero;

    ret = uintbig_const_one;
    for(idig=b->ndig-1; idig>=0; --idig)
    {
        uint8_t mask = 0x80;

        for(ibit=8; ibit; --ibit, mask>>=1)
        {
            ret = uintbig_power2(&ret);
            if( b->digit[idig] & mask )
                ret = uintbig_mul(&ret, a);
        }
    }

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_powermod(const uintbig_t *a, const uintbig_t *b, const uintbig_t *n)
{
    /**
     * Calculate 'a'^'b' modulo 'n', 'a' to the power of 'b' for the remainder of 'n'.
     */
    uintbig_t ret;
    int       idig, ibit;

    assert( a && b && n );

    if( a->ndig <= 0 ||
        b->ndig <= 0 ||
        n->ndig <= 0 )
    {
        return uintbig_const_nan;
    }
    if( uintbig_is_zero(n) ) return uintbig_const_nan;
    if( uintbig_is_zero(b) ) return uintbig_const_one;
    if( uintbig_is_zero(a) ) return uintbig_const_zero;

    ret = uintbig_const_one;
    for(idig=b->ndig-1; idig>=0; --idig)
    {
        uint8_t mask = 0x80;

        for(ibit=8; ibit; --ibit, mask>>=1)
        {
            ret = uintbig_power2(&ret);
            ret = uintbig_mod(&ret, n);
            if( b->digit[idig] & mask )
            {
                ret = uintbig_mul(&ret, a);
                ret = uintbig_mod(&ret, n);
            }
        }
    }

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_sqrt(const uintbig_t *a)
{
    /**
     * Calculate the square root of 'a'
     *
     * To see more details about the algorithm, go to this website:
     * http://en.wikipedia.org/wiki/Methods_of_computing_square_roots
     */
    uintbig_t num;
    uintbig_t bit;
    uintbig_t res;

    assert( a );

    // Special cases
    if( a->ndig <= 0       ) return uintbig_const_nan;
    if( uintbig_is_zero(a) ) return uintbig_const_zero;

    // Make 'num' to the copy of 'a'
    num = *a;
    // Set 'bit' to the highest power of four so that 'bit' be less equal to 'a'.
    {
        int dig;
        int digsrc = num.digit[ num.ndig - 1 ];
        for(dig=0x40; dig>digsrc; dig>>=2);

        bit                       = uintbig_const_zero;
        bit.ndig                  = num.ndig;
        bit.digit[ bit.ndig - 1 ] = dig;
    }
    // Solve the result
    res = uintbig_const_zero;
    while( !uintbig_is_zero(&bit) )
    {
        uintbig_t temp;

        temp = uintbig_add(&res, &bit);
        if( uintbig_is_greatequal(&num, &temp) )
        {
            num = uintbig_sub(&num, &temp);
            uintbig_shift_right(&res, 1);
            res = uintbig_add(&res, &bit);
        }
        else
        {
            uintbig_shift_right(&res, 1);
        }
        uintbig_shift_right(&bit, 2);
    }

    return res;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_gcd(const uintbig_t *a, const uintbig_t *b)
{
    /**
     * Calculate the greatest common divisor of 'a' and 'b'
     */
    uintbig_t  obja = *a;
    uintbig_t  objb = *b;
    uintbig_t *pa   = &obja;
    uintbig_t *pb   = &objb;
    uintbig_t *temp;

    assert( a && b );

    if( a->ndig <=0 || b->ndig <= 0 ) return uintbig_const_nan;
    if( uintbig_is_equal(a,b)       ) return *a;

    while( !uintbig_is_zero(pb) )
    {
        temp = pa;
        pa   = pb;
        pb   = temp;

        *pb  = uintbig_mod(pb, pa);
    }

    return *pa;
}
//------------------------------------------------------------------------------
bool uintbig_modmulinv(const uintbig_t *a, const uintbig_t *n, uintbig_t *inv)
{
    /**
     * Calculate the modular multiplicative inverse of 'a',
     * so that ( 'a' * 'inv' ) % 'n' == 1.
     * @a      : Parameter 'a'
     * @n      : Parameter 'n'
     * @inv    : Return the modular multiplicative inverse solved by this function.
     * @return : TRUE if succeeded;
     *           and FALSE if the modular multiplicative inverse does not existed.
     * Note : Extended Euclidean algorithm used in this function.
     */
    uintbig_t obj1, obj2, obj3, obj4, obj5, obj6, obj7;
    uintbig_t *rem1 = &obj1;
    uintbig_t *rem2 = &obj2;
    uintbig_t *rem3 = &obj3;
    uintbig_t *aux1 = &obj4;
    uintbig_t *aux2 = &obj5;
    uintbig_t *aux3 = &obj6;
    uintbig_t *quo  = &obj7;
    bool       succeed;

    assert( a && n && inv );

    if( a->ndig <=0 || n->ndig <= 0 )
    {
        *inv = uintbig_const_nan;
        return false;
    }

    *rem1 = *n;
    *rem2 =
    *rem3 = *a;
    *aux1 = uintbig_const_zero;
    *aux2 = uintbig_const_one;
    while( uintbig_is_greatthen(rem2, &uintbig_const_one) )
    {
        uintbig_t  temp;
        uintbig_t *swptmp;

        uintbig_divmod(rem1, rem2, quo, rem3);
        temp  = uintbig_mul(aux2, quo);
        *aux3 = uintbig_sub(aux1, &temp);

        swptmp = aux1;
        aux1   = aux2;
        aux2   = aux3;
        aux3   = swptmp;

        swptmp = rem1;
        rem1   = rem2;
        rem2   = rem3;
        rem3   = swptmp;
    }

    succeed = !uintbig_is_zero(rem2) && aux2->digit[BIGNUM_DIGSZ-1] < 0x80;
    if( succeed ) *inv = *aux2;

    return succeed;
}
//------------------------------------------------------------------------------
//---- Random Number and Prime Number ------------------------------------------
//------------------------------------------------------------------------------
uintbig_t uintbig_rand_ndig(unsigned int ndig, mt_rand64_t *rndobj)
{
    /**
     * Generate an 'ndig' digits random number object
     * @ndig   : Number of digits to the random number
     * @return : The random number generated
     * Note : It is recommended to call 'mt_srand' before using this function.
     */
    int        nrnds, nzeros;
    uint8_t   *digbuf;
    uintbig_t  ret;

    nrnds  = MIN( ndig , BIGNUM_DIGSZ );
    nzeros = BIGNUM_DIGSZ - nrnds;

    mt_randbin(rndobj, ret.digit, nrnds);
    digbuf = ret.digit + nrnds;
    while( nzeros-- )
    {
        *digbuf++ = 0;
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_rand_range(const uintbig_t *minval, const uintbig_t *maxval, mt_rand64_t *rndobj)
{
    /**
     * Generate a random number in a range
     * @minval : The minimum number of the range
     * @maxval : The maximum number of the range
     * @return : The random number generated, and minval <= result < maxval.
     * Note : It is recommended to call 'mt_srand' before using this function.
     */
    int       ndig;
    uintbig_t range;
    uintbig_t ret;

    assert( minval && maxval );

    range = uintbig_sub(maxval, minval);
    if( range.ndig <= 0 ) return uintbig_const_nan;

    ndig = mt_rand64(rndobj) % range.ndig;

    ret = uintbig_rand_ndig(ndig, rndobj);
    ret = uintbig_mod(&ret, &range);
    ret = uintbig_add(&ret, minval);

    return ret;
}
//------------------------------------------------------------------------------
static
void separate_value_to_the_power_combination_of_two(const uintbig_t /* in*/ *n,
                                                    uintbig_t       /*out*/ *d,
                                                    int             /*out*/ *s)
{
    /**
     * Analysis 'n' and solve 'd' and 's' so that n-1=d*2^s
     * WARNING : 'n' must be odd.
     */
    int     i;
    uint8_t trailingbyte;

    assert( n && d && s );

    // Odd to even
    *d          = *n;
    d->digit[0] = d->digit[0] & 0xFE;

    // Count the tariling zero bytes
    for(( i=0 );( i < d->ndig && !d->digit[i] );( ++i )) ;
    uintbig_byteshift_right(d, i);

    // Count the tariling zero bits
    i <<= 3;  // bytes to bits
    trailingbyte = d->digit[0];
    while( !( trailingbyte & 0x01 ) )
    {
        ++i;
        trailingbyte >>= 1;
    }
    uintbig_shift_right(d, i & 0x07);

    // Save value 's'
    *s = i;
}
//------------------------------------------------------------------------------
static
bool miller_rabin_test(const uintbig_t *n,
                       const uintbig_t *ns1,
                       const uintbig_t *d,
                       int              s,
                       const uintbig_t *a)
{
    /**
     * Miller-Rabin primality test
     * @n      : A value to test if it is a prime number.
     * @ns1    : The same value as n-1.
     * @d      : A component of 'n', so that n-1=d*2^s.
     * @s      : A component of 'n', so that n-1=d*2^s.
     * @a      : A random value that 2 <= a <= n-2.
     * @return : TRUE if the input value probably a prime number,
     *           and FALSE if it is a composite value.
     *
     * Pseudo code :
     *
     *   note : a^b means a to the power of b, not xor.
     *
     *   input n, n > 3 and n is odd
     *   input d and s, and n-1=d*2^s
     *   input a, a is a random number, and 2 <= a <= n-2
     *
     *   x = a^d mod n
     *   if x=1 or x=n-1, result "Probably Prime"
     *   run s-1 times {
     *       x = x^2 mod n
     *       if x=1, result "Composite"
     *       if x=n-1, result "Probably Prime"
     *   }
     *   result "Composite"
     */
    uintbig_t x;

    assert( n && ns1 && d && a );

    x = uintbig_powermod(a, d, n);
    if( uintbig_is_equal(&x, &uintbig_const_one) ||
        uintbig_is_equal(&x,  ns1              ) )
    {
        return true;
    }

    while( --s )
    {
        x = uintbig_power2(&x);
        x = uintbig_mod(&x, n);
        if( uintbig_is_equal(&x, &uintbig_const_one) ) return false;
        if( uintbig_is_equal(&x,  ns1              ) ) return true;
    }

    return false;
}
//------------------------------------------------------------------------------
bool uintbig_prime_test(const uintbig_t *val, unsigned int times, mt_rand64_t *rndobj)
{
    /**
     * Determine that if the input value is a prime number
     * @val    : A value to test
     * @times  : Times of the test process to run, a large value leading to a
     *           more accurate result but incresed the test time.
     * @return : TRUE if the input value probably a prime number,
     *           and FALSE if it is a composite value.
     * Note : This function test the input value by Miller–Rabin primality test.
     */
    uintbig_t d;
    int       s;
    uintbig_t a;
    uintbig_t n_sub_1;

    assert( val );

    // Special cases
    if( val->ndig <= 0 ) return false;
    if( uintbig_is_lessequal(val, &uintbig_const_three) )
    {
        return uintbig_is_equal(val, &uintbig_const_two  ) ||
               uintbig_is_equal(val, &uintbig_const_three);
    }
    if( !( val->digit[0] & 0x01 ) ) return false;

    // Separate the analyte
    separate_value_to_the_power_combination_of_two(val, &d, &s);

    // Witness loop
    n_sub_1 = *val;
    uintbig_minusminus(&n_sub_1);
    while( times-- )
    {
        a = uintbig_rand_range(&uintbig_const_two, &n_sub_1, rndobj);
        if( !miller_rabin_test(val, &n_sub_1, &d, s, &a) )
            return false;
    }

    return true;
}
//------------------------------------------------------------------------------

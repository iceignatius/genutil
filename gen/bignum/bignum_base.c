#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../minmax.h"
#include "./const.h"
#include "./base.h"

//------------------------------------------------------------------------------
int uintbig_get_digit_usage(const uint8_t digit[BIGNUM_DIGSZ])
{
    /**
     * 計算數值資料實際使用的位元組數
     */
    int cnt;

    assert( digit );

    digit += BIGNUM_DIGSZ - 1;
    for(cnt=BIGNUM_DIGSZ; cnt>1; --cnt, --digit)
    {
        if( *digit ) break;
    }

    return cnt;
}
//------------------------------------------------------------------------------
void uintbig_set_dig(uintbig_t *obj, uint8_t val)
{
    assert( obj );

    memset(obj->digit, 0, BIGNUM_DIGSZ);
    obj->digit[0] = val;
    obj->ndig     = 1;
}
//------------------------------------------------------------------------------
bool uintbig_is_zero(const uintbig_t *obj)
{
    assert( obj );
    return ( obj->ndig == 1 )&&( obj->digit[0] == 0 );
}
//------------------------------------------------------------------------------
bool uintbig_is_nan(const uintbig_t *obj)
{
    assert( obj );
    return obj->ndig == -1;
}
//------------------------------------------------------------------------------
bool uintbig_is_equal(const uintbig_t *obj1, const uintbig_t *obj2)
{
    assert( obj1 && obj2 );

    if( obj1->ndig != obj2->ndig ) return false;
    if( obj1->ndig <= 0 ) return true;
    return !memcmp(obj1->digit, obj2->digit, obj1->ndig );
}
//------------------------------------------------------------------------------
bool uintbig_is_greatthen(const uintbig_t *obj1, const uintbig_t *obj2)
{
    int i;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return false;
    if( obj1->ndig != obj2->ndig ) return obj1->ndig > obj2->ndig;
    for(i=obj1->ndig-1; i>=0; --i)
    {
        if( obj1->digit[i] == obj2->digit[i] ) continue;
        return obj1->digit[i] > obj2->digit[i];
    }

    return false;
}
//------------------------------------------------------------------------------
bool uintbig_is_lessthen(const uintbig_t *obj1, const uintbig_t *obj2)
{
    int i;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return false;
    if( obj1->ndig != obj2->ndig ) return obj1->ndig < obj2->ndig;
    for(i=obj1->ndig-1; i>=0; --i)
    {
        if( obj1->digit[i] == obj2->digit[i] ) continue;
        return obj1->digit[i] < obj2->digit[i];
    }

    return false;
}
//------------------------------------------------------------------------------
bool uintbig_is_greatequal(const uintbig_t *obj1, const uintbig_t *obj2)
{
    int i;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return false;
    if( obj1->ndig != obj2->ndig ) return obj1->ndig > obj2->ndig;
    if( !memcmp(obj1->digit, obj2->digit, obj1->ndig) ) return true;
    for(i=obj1->ndig-1; i>=0; --i)
    {
        if( obj1->digit[i] == obj2->digit[i] ) continue;
        return obj1->digit[i] > obj2->digit[i];
    }

    return false;
}
//------------------------------------------------------------------------------
bool uintbig_is_lessequal (const uintbig_t *obj1, const uintbig_t *obj2)
{
    int i;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return false;
    if( obj1->ndig != obj2->ndig ) return obj1->ndig < obj2->ndig;
    if( !memcmp(obj1->digit, obj2->digit, obj1->ndig) ) return true;
    for(i=obj1->ndig-1; i>=0; --i)
    {
        if( obj1->digit[i] == obj2->digit[i] ) continue;
        return obj1->digit[i] < obj2->digit[i];
    }

    return false;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_and(const uintbig_t *obj1, const uintbig_t *obj2)
{
    uintbig_t ret;
    int       ndig;
    int       i = BIGNUM_DIGSZ;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return uintbig_const_nan;

    ndig = MIN( obj1->ndig , obj2->ndig );
    while( i > ndig )
    {
        ret.digit[--i] = 0;
    }
    while( i-- )
    {
        ret.digit[i] = obj1->digit[i] & obj2->digit[i];
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_or(const uintbig_t *obj1, const uintbig_t *obj2)
{
    uintbig_t ret;
    int       ndig;
    int       i = BIGNUM_DIGSZ;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return uintbig_const_nan;

    ndig = MAX( obj1->ndig , obj2->ndig );
    while( i > ndig )
    {
        ret.digit[--i] = 0;
    }
    while( i-- )
    {
        ret.digit[i] = obj1->digit[i] | obj2->digit[i];
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_xor(const uintbig_t *obj1, const uintbig_t *obj2)
{
    uintbig_t ret;
    int       ndig;
    int       i = BIGNUM_DIGSZ;

    assert( obj1 && obj2 );

    if( obj1->ndig <= 0 || obj2->ndig <= 0 ) return uintbig_const_nan;

    ndig = MAX( obj1->ndig , obj2->ndig );
    while( i > ndig )
    {
        ret.digit[--i] = 0;
    }
    while( i-- )
    {
        ret.digit[i] = obj1->digit[i] ^ obj2->digit[i];
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
void uintbig_byteshift_left(uintbig_t *obj, unsigned int n)
{
    int i;

    assert( obj );

    if( n == 0 ) return;
    if( obj->ndig <= 0 ) return;
    if( uintbig_is_zero(obj) ) return;
    if( n > BIGNUM_DIGSZ )
    {
        *obj = uintbig_const_zero;
        return;
    }

    // Shift digits
    for(i = MIN( BIGNUM_DIGSZ , obj->ndig + n ) - 1;
        i >= (int)n;
        --i)
    {
        obj->digit[i] = obj->digit[i-n];
    }
    while( i >= 0 )
    {
        obj->digit[i--] = 0;
    }

    // Update digit usage count
    obj->ndig = MIN( BIGNUM_DIGSZ, obj->ndig + n );
}
//------------------------------------------------------------------------------
void uintbig_byteshift_right(uintbig_t *obj, unsigned int n)
{
    int i;

    assert( obj );

    if( n == 0 ) return;
    if( obj->ndig <= 0 ) return;
    if( (int)n >= obj->ndig )
    {
        *obj = uintbig_const_zero;
        return;
    }

    // Shift digits
    for(i = 0;
        i < (int)( obj->ndig - n );
        ++i)
    {
        obj->digit[i] = obj->digit[i+n];
    }
    while( i < obj->ndig )
    {
        obj->digit[i++] = 0;
    }

    // Update digit usage count
    obj->ndig -= n;
}
//------------------------------------------------------------------------------
void uintbig_shift_left(uintbig_t *obj, unsigned int n)
{
    unsigned int nbyte = n >> 3;    // nbyte = n/8
    unsigned int nbit  = n & 0x07;  // nbit  = n%8
    int          i_head, i;

    assert( obj );
    if( obj->ndig <= 0 ) return;

    // Shift bytes
    uintbig_byteshift_left(obj, nbyte);
    if( nbit == 0 ) return;

    // Shift bits of digits
    i_head = MIN( obj->ndig, BIGNUM_DIGSZ - 1 );
    for(i=i_head; i>0; --i)
        obj->digit[i] = ( obj->digit[i] << nbit ) | ( obj->digit[i-1] >> (8-nbit) );
    // - Do not forget the last digit
    obj->digit[0] <<= nbit;

    // Update digit usage count
    if( obj->digit[i_head] )
        obj->ndig = MIN( obj->ndig + 1 , BIGNUM_DIGSZ );
}
//------------------------------------------------------------------------------
void uintbig_shift_right(uintbig_t *obj, unsigned int n)
{
    unsigned int nbyte = n >> 3;    // nbyte = n/8
    unsigned int nbit  = n & 0x07;  // nbit  = n%8
    int          i_head, i;

    assert( obj );
    if( obj->ndig <= 0 ) return;

    // Shift bytes
    uintbig_byteshift_right(obj, nbyte);
    if( nbit == 0 ) return;

    // Shift bits of digits
    i_head = MIN(obj->ndig, BIGNUM_DIGSZ) - 1;
    for(i=0; i<obj->ndig-1; ++i)
        obj->digit[i] = ( obj->digit[i] >> nbit ) | ( obj->digit[i+1] << (8-nbit) );
    // - Do not forget the last digit
    obj->digit[i_head] >>= nbit;

    // Update digit usage count
    if( obj->ndig > 1 && !obj->digit[i_head] )
        --obj->ndig;
}
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../inline.h"
#include "../minmax.h"
#include "./const.h"
#include "./io.h"

//------------------------------------------------------------------------------
uintbig_t uintbig_from_bin(const void *src, size_t size)
{
    /**
     * 從二進位位元組資料匯入至物件
     * @src    : 欲匯入的位元組資料，資料內容以 Little-Endian 的格式被解讀。
     * @size   : 欲匯入的資料大小；
     *           物件無法容納的資料將會被截斷(從數位較大的一端截斷)。
     * @return : 傳回含有數值資訊的物件；
     *           若無任何資料傳入則數值為零的物件將會被傳回。
     */
    uintbig_t      ret;
    const uint8_t *srcbuf  = src;
    uint8_t       *destbuf = ret.digit;
    size_t         ndat, nzeros;

    if( !src || !size ) return uintbig_const_zero;

    ndat   = MIN( size , BIGNUM_DIGSZ );
    nzeros = BIGNUM_DIGSZ - ndat;

    while( ndat-- )
        *destbuf++ = *srcbuf++;
    while( nzeros-- )
        *destbuf++ = 0;

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uintbig_t uintbig_from_int(uint64_t val)
{
    /**
     * 從整數匯入資料至物件
     * @val    : 欲匯入的整數
     * @return : 傳回含有數值資訊的物件
     */
    uintbig_t ret = uintbig_const_zero;
    int       i;

    for(i=0; i<sizeof(val) && i<BIGNUM_DIGSZ; ++i)
    {
        int off = i<<3;
        ret.digit[i] = ( val & ( 0xFF << off ) ) >> off;
    }
    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
INLINE
bool is_hex_char(char c)
{
    return ( '0' <= c && c <= '9' )||
           ( 'A' <= c && c <= 'F' )||
           ( 'a' <= c && c <= 'f' );
}
//------------------------------------------------------------------------------
static
uint8_t hexch_to_num(char c)
{
    if     ( '0' <= c && c <= '9' ) return c - '0';
    else if( 'A' <= c && c <= 'F' ) return c - 'A' + 10;
    else if( 'a' <= c && c <= 'f' ) return c - 'a' + 10;
    else                            return 0;
}
//------------------------------------------------------------------------------
INLINE
uint8_t hexchs_to_bin(char c1, char c0)
{
    return ( hexch_to_num(c1) << 4 ) | hexch_to_num(c0);
}
//------------------------------------------------------------------------------
uintbig_t uintbig_from_hex(const char *str)
{
    /**
     * 讀取字串表達的數值並匯入物件
     * @str    : 來源字串，並注意：
     *           1. 字串分析與轉換將從第一個可辨識字元起始，
     *              至開始便是後的第一個不可辨識字元或字串結尾中止。
     *           2. 目前只支援 Hexadecimal 格式
     *           3. 物件無法容納的數值將會被截斷(從數位較大的一端截斷)
     * @return : 轉換成功傳回含有數值資訊的物件；失敗時傳回意義為 NAN 的物件。
     */
    const char *str0;
    const char *str1;
    uintbig_t   ret;
    uint8_t    *digbuf;

    // Initialize object
    ret = uintbig_const_nan;

    if( !str ) return ret;

    // Find string start
    str0 = str;
    while( *str0 && !is_hex_char(*str0) ) ++str0;
    if( *str0 == 0 ) return ret;
    // Find string end
    str1 = str0;
    while( *str1 && is_hex_char(*str1) ) ++ str1;
    // Trim string start position
    if( str1 - str0 > 2*BIGNUM_DIGSZ )
        str0 = str1 - 2*BIGNUM_DIGSZ;

    // Translate Hexadecimal text to binary data
    for(( digbuf=ret.digit, str=str1-2 );( str>=str0 );( str-=2, ++digbuf ))
    {
        *digbuf = hexchs_to_bin(str[0], str[1]);
    }
    if( ++str == str0 )
    {
        *digbuf = hexchs_to_bin(0, *str);
    }

    ret.ndig = uintbig_get_digit_usage(ret.digit);

    return ret;
}
//------------------------------------------------------------------------------
uint64_t uintbig_to_int(const uintbig_t *obj)
{
    /**
     * 將物件內容數值匯出為整數
     * @obj    : 欲做資料匯出的物件
     * @return : 傳回的整數資料，並注意：
     *           1. 無論物件內容是否可用都會將資料傳回
     *           2. 超出整數範圍的資料將會被截斷
     */
#if BIGNUM_DIGSZ < 8
    int      n = MIN( sizeof(uint64_t) , MAX(obj->ndig, 0) );
#else
    int      n = sizeof(uint64_t);
#endif
    uint64_t ret = 0;
    int      i;

    assert( obj );

    for(i=0; i<n; ++i)
        ret |= ( (uint64_t)obj->digit[i] ) << ( i << 3 );

    return ret;
}
//------------------------------------------------------------------------------
size_t uintbig_to_str(char *buf, size_t bufsz, const uintbig_t *obj)
{
    /**
     * 將物件內容數值轉換為字串
     * @buf    : 接收轉換後字串的緩衝區
     * @bufsz  : 緩衝區大小
     * @obj    : 物件
     * @return : 轉換成功時傳回寫入緩衝區的字元組數；轉換失敗傳回零。
     * Note : 目前只支援 Hexadecimal 格式
     */
    size_t wcnt;
    int    i;

    if( !buf || !obj ) return 0;

    if( obj->ndig > 0 )
    {
        wcnt = ( obj->ndig << 1 ) + 1;
        if( bufsz < wcnt ) return 0;

        for(i=obj->ndig-1; i>=0; --i, buf+=2)
            sprintf(buf, "%2.2X", obj->digit[i]);
    }
    else
    {
        switch( obj->ndig )
        {
        case -1:
            wcnt = strlen("NAN") + 1;
            if( bufsz < wcnt ) return 0;
            strcpy(buf, "NAN");
            break;
        default:
            return 0;
        }
    }

    return wcnt;
}
//------------------------------------------------------------------------------

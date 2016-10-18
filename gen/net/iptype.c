#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ipconst.h"
#include "iptype.h"

#ifdef _MSC_VER
    #define snprintf _snprintf
#endif

#define MAXSTRLEN (0xFF)

//------------------------------------------------------------------------------
static
char* strcat_ch(char *dest, char ch)
{
    if( dest )
    {
        size_t len = strlen(dest);
        dest[len  ] = ch;
        dest[len+1] = 0;
    }

    return dest;
}
//------------------------------------------------------------------------------
//---- IPv4 --------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t ipv4_to_str(char* dest, size_t destsz, ipv4_t ip)
{
    /**
     * 將 IP 寫為字串。
     *
     * @param dest   字串寫入的緩衝區，此值可為 NULL。
     * @param destsz @a dest 的大小。
     * @param ip     傳入欲做轉換 ipv4_t 物件。
     * @return 寫入成功時傳回寫入 dest 的資料大小；
     *         寫入失敗時傳回零；
     *         若 dest 為 NULL 則傳回 dest 應具有的最小大小。
     */
    char    temp[16];
    char   *buf   = ( dest )?( dest   ):( temp         );
    size_t  bufsz = ( dest )?( destsz ):( sizeof(temp) );
    int     recsz;

    recsz = snprintf(buf,
                     bufsz,
                     "%d.%d.%d.%d",
                     (int)ip.dig[0],
                     (int)ip.dig[1],
                     (int)ip.dig[2],
                     (int)ip.dig[3]);
    assert( recsz > 0 );

    return recsz + 1;
}
//------------------------------------------------------------------------------
static
bool full_str_to_uint(const char *str, unsigned long *result)
{
    int   base;
    char *end;

    assert( str && result );

    base = strncmp(str, "0x", 2) ? 10 : 16;
    if( base == 16 ) str += 2;

    *result = strtoul(str, &end, base);

    return !(*end);
}
//------------------------------------------------------------------------------
static
ipv4_t ipv4_from_dotted_str(const char *str)
{
    char  buf[24] = {0};
    char *valstr;
    unsigned long dig0, dig1, dig2, dig3;

    assert( str );

    strncpy(buf, str, sizeof(buf)-1);

    valstr = strtok(buf, ".");
    if( !valstr || !full_str_to_uint(valstr, &dig0) ) return ipv4_const_invalid;

    valstr = strtok(NULL, ".");
    if( !valstr || !full_str_to_uint(valstr, &dig1) ) return ipv4_const_invalid;

    valstr = strtok(NULL, ".");
    if( !valstr || !full_str_to_uint(valstr, &dig2) ) return ipv4_const_invalid;

    valstr = strtok(NULL, ".");
    if( !valstr || !full_str_to_uint(valstr, &dig3) ) return ipv4_const_invalid;

    return ipv4_from_digits(dig0, dig1, dig2, dig3);
}
//------------------------------------------------------------------------------
static
ipv4_t ipv4_from_integer_str(const char *str)
{
    unsigned long ipval;

    assert( str );

    return full_str_to_uint(str, &ipval) ? ipv4_from_int(ipval) : ipv4_const_invalid;
}
//------------------------------------------------------------------------------
ipv4_t ipv4_from_str(const char *str)
{
    /**
     * 解析字串型態表現的 IP，並轉換為 ipv4_t 型態。
     *
     * @param str 傳入 IP 字串。
     * @return 解析成功傳回以 ipv4_t 型態表示的 IP；解析失敗傳回零值的 IP。
     */
    if( !str ) return ipv4_const_invalid;

    return strchr(str, '.') ? ipv4_from_dotted_str(str) : ipv4_from_integer_str(str);
}
//------------------------------------------------------------------------------
//---- IPv6 --------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t ipv6_to_str_full(char* dest, size_t destsz, ipv6_t ip)
{
    /**
     * 將 IP 寫為字串(IPv6 完整表示)。
     *
     * @param dest   字串寫入的緩衝區，此值可為 NULL。
     * @param destsz @a dest 的大小。
     * @param ip     傳入欲做轉換 ipv6_t 物件。
     * @return 寫入成功時傳回寫入 dest 的資料大小；
     *         寫入失敗時傳回零；
     *         若 dest 為 NULL 則傳回 dest 應具有的最小大小。
     */
    char    temp[40];
    char   *buf   = ( dest )?( dest   ):( temp         );
    size_t  bufsz = ( dest )?( destsz ):( sizeof(temp) );
    int     recsz;

    recsz = snprintf(buf,
                     bufsz,
                     "%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X",
                     ipv6_get_digit(ip, 0),
                     ipv6_get_digit(ip, 1),
                     ipv6_get_digit(ip, 2),
                     ipv6_get_digit(ip, 3),
                     ipv6_get_digit(ip, 4),
                     ipv6_get_digit(ip, 5),
                     ipv6_get_digit(ip, 6),
                     ipv6_get_digit(ip, 7));
    assert( recsz > 0 );

    return recsz + 1;
}
//------------------------------------------------------------------------------
size_t ipv6_to_str_short(char* dest, size_t destsz, ipv6_t ip)
{
    /**
     * 將 IP 寫為字串(IPv6 縮短表示)。
     *
     * @param dest   字串寫入的緩衝區，此值可為 NULL。
     * @param destsz @a dest 的大小。
     * @param ip     傳入欲做轉換 ipv6_t 物件。
     * @return 寫入成功時傳回寫入 dest 的資料大小；
     *         寫入失敗時傳回零；
     *         若 dest 為 NULL 則傳回 dest 應具有的最小大小。
     */
    int i;
    // - Veribales for : Get digits
    int digit[8];
    // - Veriables for : Zeros analysis
    int zeros[8];  // zeros[i] 記錄 digit[i] 之後(含其本身)有多少連續的 digits 皆為零
    int zeros_maxval = 0;
    int zeros_maxloc = -1;
    // - Veribales for : Print
    char buffer[40];
    int  prtres;
    int  prtlen = 0;
    // - Veribales for : Copy data
    size_t recsz;

    // Get digits
    for(i=0; i<8; ++i) digit[i] = ipv6_get_digit(ip, i);

    // Zeros analysis
    for(i=0; i<8; ++i) zeros[i] = ( digit[i] == 0 )?( 1 ):( 0 );
    for(i=8-1-1; i>=0; --i)
    {
        if( zeros[i] ) zeros[i] += zeros[i+1];
    }
    for(i=0; i<8; ++i)
    {
        if( zeros[i] > zeros_maxval )
        {
            zeros_maxval = zeros[i];
            zeros_maxloc = i;
        }
    }

    // Print
    for(i=0; i<8; ++i)
    {
        if( i == zeros_maxloc )
        {
            prtres  = snprintf(buffer+prtlen, sizeof(buffer)-prtlen, ( i+zeros[i] >= 8 )?( "::" ):( ":" ));
            assert( prtres > 0 );
            prtlen += prtres;
            i      += zeros[zeros_maxloc] - 1;
        }
        else
        {
            prtres  = snprintf(buffer+prtlen, sizeof(buffer)-prtlen, (i==0)?("%X"):(":%X"), digit[i]);
            assert( prtres > 0 );
            prtlen += prtres;
        }
    }

    // Copy data to destination buffer
    recsz = strlen(buffer) + 1;
    if( dest )
    {
        if( destsz < recsz ) return 0;
        memcpy(dest, buffer, recsz);
    }

    return recsz;
}
//------------------------------------------------------------------------------
ipv6_t ipv6_from_str(const char* str)
{
    /**
     * 解析字串型態表現的 IP，並轉換為 ipv6_t 型態。
     *
     * @param str 傳入 IP 字串。
     * @return 解析成功傳回以 ipv6_t 型態表示的 IP；解析失敗傳回零值的 IP。
     */
    char block_str[8][MAXSTRLEN];
    int  block_cnt = 0;
    int  digit_int[8];
    // - Variables for : Separate sub strings
    const char *buf;
    int         buflen;
    // - Variables for : Translate IPv4 value
    bool   have_ipv4_str;
    ipv4_t ip4val;
    // - Variables for : Translate digits
    int digit_cnt = 0;
    int block_ind;

    if( !str )
        return ipv6_const_invalid;
    if( strlen(str) >= MAXSTRLEN )
        return ipv6_const_invalid;

    // Try to solve the problem with IPv4 format if the string format is not IPv6
    if( !strchr(str,':') )
        return ipv6_from_ipv4(ipv4_from_str(str));

    // Separate sub strings
    buf    = str;
    buflen = strlen(str) + 1;
    memset(block_str, 0, sizeof(block_str));
    // - Check if the string head is "::"
    if( buf[0] == ':' && buf[1] == ':' )
    {
        block_str[0][0] = 0;
        block_cnt       = 1;
        buf            += 2;
        buflen         -= 2;
    }
    // - Normal sub string separate
    while( buflen > 0 )
    {
        if( buf[0] == ':' && buf[1] == ':' )
        {
            block_str[block_cnt+1][0] = 0;
            block_cnt += 2;
            buf       += 2;
            buflen    -= 2;
        }
        else if( buf[0] == ':' || buf[0] == 0 )
        {
            ++block_cnt;
            ++buf;
            --buflen;
        }
        else
        {
            strcat_ch(block_str[block_cnt], buf[0]);
            ++buf;
            --buflen;
        }
    }
    // - Check if the string tail is "::"
    if(( block_cnt >= 2 )&&( !block_str[block_cnt-1][0] )&&( !block_str[block_cnt-2][0] ))
        --block_cnt;

    // Translate IPv4 value if the last sub string is IPv4 format
    have_ipv4_str = !!strchr(block_str[block_cnt-1], '.');
    if( have_ipv4_str ) ip4val = ipv4_from_str(block_str[--block_cnt]);

    // Translate digits
    // - Prepare
    if( digit_cnt > (have_ipv4_str?6:8) )
        return ipv6_const_invalid;
    memset(digit_int, 0, sizeof(digit_int));
    // - Translate IPv6 digits
    for(block_ind=0; block_ind<block_cnt; ++block_ind)
    {
        if( block_str[block_ind][0] == 0 )
        {
            int ZerosCnt = (have_ipv4_str?6:8) - block_cnt + 1;
            digit_cnt += ZerosCnt;
        }
        else
        {
            int ScanResult = sscanf(block_str[block_ind], "%X", &digit_int[digit_cnt++]);
            if(( ScanResult == 0 )||( ScanResult == EOF ))
                return ipv6_const_invalid;
        }
    }
    // - Set IPv4 digits
    if( have_ipv4_str )
    {
        digit_int[6] = ( ip4val.dig[0] << 8 ) | ip4val.dig[1];
        digit_int[7] = ( ip4val.dig[2] << 8 ) | ip4val.dig[3];
        digit_cnt   += 2;
    }
    // - Check digits count
    if( digit_cnt != 8 )
        return ipv6_const_invalid;

    return ipv6_from_digits(digit_int[0], digit_int[1], digit_int[2], digit_int[3],
                            digit_int[4], digit_int[5], digit_int[6], digit_int[7]);
}
//------------------------------------------------------------------------------
//---- MAC Address -------------------------------------------------------------
//------------------------------------------------------------------------------
size_t macaddr_to_str(char* dest, size_t destsz, macaddr_t addr)
{
    /**
     * 將 MAC Address 寫為字串。
     *
     * @param dest   字串寫入的緩衝區，此值可為 NULL。
     * @param destsz @a dest 的大小。
     * @param addr   傳入欲做轉換 macaddr_t 物件。
     * @return 寫入成功時傳回寫入 dest 的資料大小；
     *         寫入失敗時傳回零；
     *         若 dest 為 NULL 則傳回 dest 應具有的最小大小。
     */
    int recres;

    if( !dest ) return 18;
    if( destsz < 18 ) return 0;

    recres = snprintf(dest,
                      destsz,
                      "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
                      addr.dig[0],
                      addr.dig[1],
                      addr.dig[2],
                      addr.dig[3],
                      addr.dig[4],
                      addr.dig[5]);
    assert( recres > 0 );

    return 18;
}
//------------------------------------------------------------------------------
macaddr_t macaddr_from_str(const char* str)
{
    /**
     * 解析字串型態表現的 MAC Address，並轉換為 macaddr_t 型態。
     *
     * @param str 傳入 MAC Address 字串。
     * @return 解析成功傳回以 macaddr_t 型態表示的 MAC Address；
     *         解析失敗傳回零值的 MAC Address (macaddr_const_invalid)。
     */
    char   digstr[6][MAXSTRLEN];
    int    digint[6];
    int    digcnt = 0;
    size_t i;

    if( !str || strlen(str) >= MAXSTRLEN )
        return macaddr_const_invalid;

    // Separate digit sub strings
    memset(digstr, 0, sizeof(digstr));
    for(i=0; i<strlen(str)+1; ++i)
    {
        char ch = str[i];

        if( ch == ':' || ch == 0 )
        {
            if( ++digcnt > 6 ) break;
        }
        else
        {
            strcat_ch(digstr[digcnt], ch);
        }
    }
    if( digcnt != 6 )
    {
        return macaddr_const_invalid;
    }

    // Convert string to integer
    for(i=0; i<6; ++i)
    {
        int scnres;

        scnres = sscanf(digstr[i], "%X", &digint[i]);
        if(( scnres == 0 )||( scnres == EOF ))
        {
            return macaddr_const_invalid;
        }
    }

    return macaddr_from_digits(digint[0], digint[1], digint[2], digint[3], digint[4], digint[5]);
}
//------------------------------------------------------------------------------

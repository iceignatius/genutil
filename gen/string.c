#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "minmax.h"
#include "type.h"
#include "string.h"

//------------------------------------------------------------------------------
static
char* tempstr_clone(const char *src)
{
    size_t  strsz;
    char   *strnew;

    if( !src ) return NULL;

    strsz  = strlen(src) + 1;
    strnew = malloc(strsz);
    memcpy(strnew, src, strsz);

    return strnew;
}
//------------------------------------------------------------------------------
static
void tempstr_release(char *tempstr)
{
    if( tempstr ) free(tempstr);
}
//------------------------------------------------------------------------------
char* str_trim_trailing_space(char *str)
{
    /**
     * Remove trailing spaces of a string.
     * @param[in,out] str The string to be operated.
     * @return The modified result, and it is the same as @a str.
     */
    size_t len = strlen(str);
    if( !len ) return str;

    char *pos = str + len - 1;
    while( pos != str && *pos == ' ' )
        --pos;

    if( *pos == ' ' )
        *pos = 0;
    else
        *(++pos) = 0;
    return str;
}
//------------------------------------------------------------------------------
char* str_trim_leading_space(char *str)
{
    /**
     * Remove leading spaces of a string.
     * @param[in,out] str The string to be operated.
     * @return The modified result, and it is the same as @a str.
     */
    char *src = str;
    char *buf = str;

    if( !str ) return str;

    while( *src == ' ' )
        ++src;
    if( src == buf ) return str;

    while( *src )
        *buf++ = *src++;
    *buf = 0;

    return str;
}
//------------------------------------------------------------------------------
static
size_t get_escapehex_value_from_str(const char *hexstr, int *value)
{
    if( hexstr[0] != '\\' || hexstr[1] != 'x' ) return 0;

    char buf[3] = {0};
    strncpy(buf, &hexstr[2], sizeof(buf)-1);

    char *endptr;
    *value = strtol(buf, &endptr, 16);

    return ( endptr != buf )?( 2 + ( endptr - buf ) ):( 0 );
}
//------------------------------------------------------------------------------
static
size_t translate_escapes_to_ctrlcode(const char *str, int *code)
{
    if( str[0] != '\\' )
    {
        *code = *str;
        return 1;
    }

    switch( str[1] )
    {
    case 'a'  :  *code = 0x07;  return 2;
    case 'b'  :  *code = 0x08;  return 2;
    case 't'  :  *code = 0x09;  return 2;
    case 'n'  :  *code = 0x0A;  return 2;
    case 'v'  :  *code = 0x0B;  return 2;
    case 'f'  :  *code = 0x0C;  return 2;
    case 'r'  :  *code = 0x0D;  return 2;
    case '\\' :  *code = 0x5C;  return 2;
    case '0'  :  *code = 0x00;  return 2;
    case '\"' :  *code = 0x22;  return 2;
    case '\'' :  *code = 0x27;  return 2;
    case '?'  :  *code = 0x3F;  return 2;

    case 'x':
        return get_escapehex_value_from_str(str, code);

    default:
        return 0;
    }
}
//------------------------------------------------------------------------------
char* str_escch_to_ctrlcode(char *str)
{
    /**
     * Convert a string that contained escape characters to a string with real control code.
     * @param[in,out] str The string to be operated.
     * @return The modified result, and it is the same as @a str,
     *         and will return NULL if translation failed.
     */
    if( !str ) return NULL;

    char *tempdest = tempstr_clone(str);
    if( !tempdest ) return NULL;

    char *dest = tempdest;
    char *src  = str;
    while( *src )
    {
        int    ctrlcode;
        size_t readlen = translate_escapes_to_ctrlcode(src, &ctrlcode);

        if( !readlen )
        {
            src = NULL;  // To indicate that the translation is failed.
            break;
        }

        *dest++  = ctrlcode;
        src     += readlen;
    }
    *dest = 0;

    dest = src ? str : NULL;
    if( dest ) strcpy(dest, tempdest);

    tempstr_release(tempdest);

    return dest;
}
//------------------------------------------------------------------------------
char* str_paste(char /*in,out*/ *dest, size_t size, const char *src, size_t offset)
{
    /**
     * Paste a string to another.
     *
     * @param dest   A string buffer to be paste.
     * @param size   size of the string buffer.
     * @param src    A string to paste to the string buffer.
     * @param offset The past offset to the string buffer in characters.
     * @return The modified result, and it is the same as @a dest.
     *
     * @remarks If the offset is large then the string length in @a dest input,
     *          Space character will be filled between the two strings.
     *
     */
    /**
        @par Example
        @verbatim
        char buf[] = "1234567890"
        str_paste(buf, sizeof(buf), "abc", 3);
        // Then, buf will be "123abc7890".
        @endverbatim
     */
    char   *buf   = dest;
    size_t  bufsz = size;
    size_t  buflen, srclen;

    if( !dest || !size || !src ) return dest;

    buflen = strlen(buf);
    srclen = strlen(src);

    // Skip leading characters
    {
        size_t head_skip = MIN( offset, buflen );
        if( head_skip > bufsz ) return dest;
        buf    += head_skip;
        bufsz  -= head_skip;
        offset -= head_skip;
        buflen -= head_skip;
    }

    // Fill spaces from the original string end to the offset position
    while( offset-- && bufsz )
    {
        *buf++ = ' ';
        --bufsz;
    }
    if( !bufsz ) return dest;

    // Copy string
    while( srclen-- && bufsz )
    {
        *buf++ = *src++;
        --bufsz;
        if( buflen ) --buflen;
    }

    // Append the Null-terminator if the string is long then original
    if( !buflen && bufsz )
    {
        *buf = 0;
    }

    return dest;
}
//------------------------------------------------------------------------------
static
char nibble_to_char(unsigned nibble)
{
    static const char table[16] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    return table[ nibble & 0x0F ];
}
//------------------------------------------------------------------------------
static
unsigned char_to_nibble(char ch)
{
    switch( ch )
    {
    case '0' :  return 0x0;
    case '1' :  return 0x1;
    case '2' :  return 0x2;
    case '3' :  return 0x3;
    case '4' :  return 0x4;
    case '5' :  return 0x5;
    case '6' :  return 0x6;
    case '7' :  return 0x7;
    case '8' :  return 0x8;
    case '9' :  return 0x9;
    case 'A' :  return 0xA;
    case 'B' :  return 0xB;
    case 'C' :  return 0xC;
    case 'D' :  return 0xD;
    case 'E' :  return 0xE;
    case 'F' :  return 0xF;
    case 'a' :  return 0xA;
    case 'b' :  return 0xB;
    case 'c' :  return 0xC;
    case 'd' :  return 0xD;
    case 'e' :  return 0xE;
    case 'f' :  return 0xF;
    default  :  return 0;
    }
}
//------------------------------------------------------------------------------
static
void byte_to_hexstr(char chs[2], byte_t src)
{
    chs[0] = nibble_to_char( src >> 4 );
    chs[1] = nibble_to_char( src & 0x0F );
}
//------------------------------------------------------------------------------
static
byte_t hexstr_to_byte(const char chs[2])
{
    return ( char_to_nibble(chs[0]) << 4 )|( char_to_nibble(chs[1]) );
}
//------------------------------------------------------------------------------
size_t str_bin_to_hexstr(char       *dest,
                         size_t      destsz,
                         const void *data,
                         size_t      datsz,
                         const char *prefix,
                         const char *postfix)
{
    /**
     * Translate binary data to hexadecimal described string.
     *
     * @param dest    A buffer to receive the result string.
     * @param destsz  Size of the output buffer.
     * @param data    The binary data.
     * @param datsz   Size of the binary data.
     * @param prefix  Decoration before each bytes of string,
     *                and can be NULL to not use.
     * @param postfix Decoration after each bytes of string,
     *                and can be NULL to not use.
     * @return Size of data filled to the output buffer if succeed;
     *         and ZERO if failed.
     */
    if( !dest || !destsz || !data || !datsz ) return 0;

    prefix  = prefix  ? prefix  : "";
    postfix = postfix ? postfix : "";
    size_t prelen  = strlen(prefix);
    size_t postlen = strlen(postfix);

    size_t destsz_required = datsz*( prelen + 2 + postlen ) + 1;
    if( destsz < destsz_required ) return 0;

    for(const byte_t *src = data; datsz--; src+=1)
    {
        memcpy(dest, prefix, prelen);
        dest += prelen;

        byte_to_hexstr(dest, *src);
        dest += 2;

        memcpy(dest, postfix, postlen);
        dest += postlen;
    }

    *dest = 0;

    return destsz_required;
}
//------------------------------------------------------------------------------
static
size_t hexstr_count_readable_chars(const char *str)
{
    size_t res = 0;
    for(; *str; ++str)
    {
        if( isxdigit(*str) )
            ++res;
    }

    return res;
}
//------------------------------------------------------------------------------
static
const char* hexstr_find_xdigit_char(const char *str)
{
    // Note : If no any digit character be bound,
    //        the return value will be the last position of @str (the null-terminator),
    //        not NULL value.
    for(; *str && !isxdigit(*str); ++str)
    {
    }

    return str;
}
//------------------------------------------------------------------------------
static
const char* hexstr_get_first_bytechs(char bytechs[2+1], const char *str)
{
    // Note : Return value is the position after the character we last read of @str.

    const char *pos = str;

    pos = hexstr_find_xdigit_char(pos);
    bytechs[0] = *pos++;

    pos = hexstr_find_xdigit_char(pos);
    bytechs[1] = *pos++;

    return pos;
}
//------------------------------------------------------------------------------
size_t str_hexstr_to_bin(void       *dest,
                         size_t      destsz,
                         const char *str,
                         bool        skip_errch)
{
    /**
     * Translate a hexadecimal described string to binary data.
     *
     * @param dest       A buffer to receive the result data.
     * @param destsz     Size of the output buffer.
     * @param str        The hexadecimal described string.
     * @param skip_errch The input characters which are not in range of
     *                   '0'~'9', 'A'~'F', or 'a'~'f' will be skipped if this flag setted;
     *                   and will cause an error result otherwise.
     * @return Size of data filled to the output buffer if succeed;
     *         and ZERO if failed.
     */
    if( !dest || !destsz || !str ) return 0;

    size_t readable_count = hexstr_count_readable_chars(str);
    if( readable_count & 0x01 ) return 0;  // Digital characters count cannot be odd number.
    if( !skip_errch && readable_count != strlen(str) ) return 0;

    size_t destsz_required = readable_count >> 1;
    if( destsz < destsz_required ) return 0;

    byte_t *outbuf = dest;
    while( *str )
    {
        char bytechs[2];
        str = hexstr_get_first_bytechs(bytechs, str);
        *outbuf++ = hexstr_to_byte(bytechs);
    }

    return destsz_required;
}
//------------------------------------------------------------------------------

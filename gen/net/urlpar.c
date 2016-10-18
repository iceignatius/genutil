#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "urlpar.h"

//------------------------------------------------------------------------------
static
const char* get_string_after_symbol(const char *str, const char *symbol)
{
    /*
     * Return a string after the specific symbol (exclude the symbol);
     * or return the original string if the symbol was not found.
     */
    assert( str && symbol );

    const char *pos = strstr(str, symbol);
    return ( pos )?( pos + strlen(symbol) ):( str );
}
//------------------------------------------------------------------------------
static
char* trim_string_by_symbol(char /*in,out*/ *str, const char *symbol)
{
    /*
     * Drop string data after the specific symbol (include the symbol);
     * or return the original string if the symbol was not found.
     */
    assert( str && symbol );

    char *pos = strstr(str, symbol);
    if( pos ) *pos = 0;

    return str;
}
//------------------------------------------------------------------------------
char* urlpar_extract_sceme(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract sceme part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    if( !strstr(url, "://") ) return NULL;

    strncpy(dest, url, destsz);
    trim_string_by_symbol(dest, "://");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_user(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract user name part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    if( !strchr(src, '@') ) return NULL;

    strncpy(dest, src, destsz);
    trim_string_by_symbol(dest, "@");
    trim_string_by_symbol(dest, ":");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_pass(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract password part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    src = get_string_after_symbol(src, ":");

    if( !strchr(src, '@') ) return NULL;

    strncpy(dest, src, destsz);
    trim_string_by_symbol(dest, "@");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_host(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract host name part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    src = get_string_after_symbol(src, "@");

    strncpy(dest, src, destsz);
    trim_string_by_symbol(dest, "/");
    trim_string_by_symbol(dest, ":");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_port(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract port part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    src = get_string_after_symbol(src, "@");

    const char *port_start = strchr(src, ':');
    const char *port_end   = strchr(src, '/');
    if( !port_start ) return NULL;
    if( port_end && port_start > port_end ) return NULL;

    strncpy(dest, port_start+1, destsz);
    trim_string_by_symbol(dest, "/");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_path(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract path part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    if( !strchr(src, '/') ) return NULL;
    src = get_string_after_symbol(src, "/");

    strncpy(dest, src, destsz);
    trim_string_by_symbol(dest, "?");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_query(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract query part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    src = get_string_after_symbol(src, "/");

    const char *query_start = strchr(src, '?');
    if( !query_start ) return NULL;

    strncpy(dest, query_start+1, destsz);
    trim_string_by_symbol(dest, "#");

    return dest;
}
//------------------------------------------------------------------------------
char* urlpar_extract_fragid(char *dest, size_t destsz, const char *url)
{
    /**
     * Extract fragment ID part from an URL.
     *
     * @param dest   The output buffer.
     * @param destsz Size of the output buffer.
     * @param url    The input URL string.
     * @return The output buffer if succeed; and NULL if failed.
     */
    if( !dest || !destsz ) return NULL;

    const char *src = url;
    src = get_string_after_symbol(src, "://");
    src = get_string_after_symbol(src, "/");

    const char *frag_start = strchr(src, '#');
    if( !frag_start ) return NULL;

    strncpy(dest, frag_start+1, destsz);

    return dest;
}
//------------------------------------------------------------------------------
static
uint16_t get_default_port_by_scheme(const char *scheme)
{
    assert( scheme );

    if     ( 0 == strcmp(scheme, "ftp"   ) ) return  20;
    else if( 0 == strcmp(scheme, "ssh"   ) ) return  22;
    else if( 0 == strcmp(scheme, "sftp"  ) ) return  22;
    else if( 0 == strcmp(scheme, "telnet") ) return  23;
    else if( 0 == strcmp(scheme, "smtp"  ) ) return  25;
    else if( 0 == strcmp(scheme, "dns"   ) ) return  53;
    else if( 0 == strcmp(scheme, "http"  ) ) return  80;
    else if( 0 == strcmp(scheme, "pop3"  ) ) return 110;
    else if( 0 == strcmp(scheme, "nntp"  ) ) return 119;
    else if( 0 == strcmp(scheme, "imap"  ) ) return 143;
    else if( 0 == strcmp(scheme, "snmp"  ) ) return 161;
    else if( 0 == strcmp(scheme, "irc"   ) ) return 194;
    else if( 0 == strcmp(scheme, "https" ) ) return 443;
    else if( 0 == strcmp(scheme, "smtps" ) ) return 465;
    else if( 0 == strcmp(scheme, "ftps"  ) ) return 990;
    else                                     return   0;
}
//------------------------------------------------------------------------------
uint16_t urlpar_get_port(const char *url)
{
    /**
     * Get the communication port.
     *
     * @param url The input URL string.
     * @return The port number of the communication if succeed; and ZERO if failed.
     */
    if( !url ) return 0;

    char portstr[8];
    if( urlpar_extract_port(portstr, sizeof(portstr), url) )
        return strtoul(portstr, NULL, 10);

    char scheme[16];
    if( urlpar_extract_sceme(scheme, sizeof(scheme), url) )
        return get_default_port_by_scheme(scheme);

    return 0;
}
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <locale.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "utf.h"

//------------------------------------------------------------------------------
//---- Character Translation ---------------------------------------------------
//------------------------------------------------------------------------------
size_t ch_utf32_to_utf16(char16_t dest[2], char32_t src)
{
    /**
     * @brief Translate a single UTF-32 code to UTF-16 data.
     *
     * @param dest 用來接收轉換結果的陣列，陣列大小為 2 以內即可，而這個輸入也可為 NULL。
     * @param src  來源字元。
     * @return 傳回被寫入陣列的元素數量，若 dest 為 NULL 則傳回 dest 陣列所需的大小。
     *
     * @note 若 src 為無效值，則函式會將其值對應至某個有效值域內，但對應方法未定義。
     */
    size_t nwrite;

    // Map the invalid code to the valid range
    src &= 0x7FFFFFFF;
    if( src > 0x10FFFF ) src %= 0x10FFFF + 1;

    // Calculate the output count
    nwrite = ( src > 0xFFFF )?( 2 ):( 1 );

    // Translate data
    if( dest )
    {
        if( nwrite == 1 )
        {
            dest[0] = src;
        }
        else
        {
            src    -= 0x10000;                    // Now 0 <= src <= 0xFFFFF (20 bits)
            dest[0] = 0xD800 | ( src >> 10 );     // The leading surrogate is 0xD800 plus the higher 10 bits of src
            dest[1] = 0xDC00 | ( src & 0x03FF );  // The trailing surrogate is 0xDC00 plus the lower 10 bits of src
        }
    }

    return nwrite;
}
//------------------------------------------------------------------------------
size_t ch_utf32_to_utf8(char dest[6], char32_t src)
{
    /**
     * @brief Translate a single UTF-32 code to UTF-8 data.
     *
     * @param dest 用來接收轉換結果的陣列，陣列大小為 6 以內即可，而這個輸入也可為 NULL。
     * @param src  來源字元。
     * @return 傳回被寫入陣列的元素數量，若 dest 為 NULL 則傳回 dest 陣列所需的大小。
     *
     * @note 若 src 為無效值，則函式會將其值對應至某個有效值域內，但對應方法未定義。
     */
    size_t nwrite;

    // Map the invalid code to the valid range
    src &= 0x7FFFFFFF;

    // Calculate the output count
    if     ( !( src & 0xFFFFFF80 ) ) nwrite   = 1;
    else if( !( src & 0xFFFFF800 ) ) nwrite   = 2;
    else if( !( src & 0xFFFF0000 ) ) nwrite   = 3;
    else if( !( src & 0xFFE00000 ) ) nwrite   = 4;
    else if( !( src & 0xFC000000 ) ) nwrite   = 5;
    else                             nwrite   = 6;

    // Translate data
    if( dest )
    {
        switch( nwrite )
        {
        case 6:
            dest[0] = 0xFC | (  src>>30 );
            dest[1] = 0x80 | ( (src>>24) & 0x3F );
            dest[2] = 0x80 | ( (src>>18) & 0x3F );
            dest[3] = 0x80 | ( (src>>12) & 0x3F );
            dest[4] = 0x80 | ( (src>> 6) & 0x3F );
            dest[5] = 0x80 | (  src      & 0x3F );
            break;

        case 5:
            dest[0] = 0xF8 | (  src>>24 );
            dest[1] = 0x80 | ( (src>>18) & 0x3F );
            dest[2] = 0x80 | ( (src>>12) & 0x3F );
            dest[3] = 0x80 | ( (src>> 6) & 0x3F );
            dest[4] = 0x80 | (  src      & 0x3F );
            break;

        case 4:
            dest[0] = 0xF0 | (  src>>18 );
            dest[1] = 0x80 | ( (src>>12) & 0x3F );
            dest[2] = 0x80 | ( (src>> 6) & 0x3F );
            dest[3] = 0x80 | (  src      & 0x3F );
            break;

        case 3:
            dest[0] = 0xE0 | (  src>>12 );
            dest[1] = 0x80 | ( (src>> 6) & 0x3F );
            dest[2] = 0x80 | (  src      & 0x3F );
            break;

        case 2:
            dest[0] = 0xC0 | ( src >> 6 );
            dest[1] = 0x80 | ( src & 0x3F );
            break;

        default:
            dest[0] = src;

        }
    }

    return nwrite;
}
//------------------------------------------------------------------------------
INLINE bool is_leading_surrogate (char16_t ch){ return ( ch & 0xFC00 ) == 0xD800; }
INLINE bool is_trailing_surrogate(char16_t ch){ return ( ch & 0xFC00 ) == 0xDC00; }
//------------------------------------------------------------------------------
size_t ch_utf16_to_utf32(char32_t* RESTRICT dest, const char16_t* RESTRICT src)
{
    /**
     * @brief Translate UTF-16 data to a single UTF-32 code.
     *
     * @param dest 用來接收轉換結果的變數，可為 NULL。
     * @param src  來源資料。
     * @return 傳回本次轉換中從來源資料陣列所取用的資料數量。
     *
     * @note 在遇到無法正確轉換的資料時，該單一資料位元組將被直接視為一個號碼作為轉換。
     */
    const uint16_t *usrc = (const uint16_t*) src;
    size_t          nread;

    // Calculate the input count
    if( !usrc )
    {
        nread = 0;
    }
    if( is_leading_surrogate(usrc[0]) && is_trailing_surrogate(usrc[1]) )
    {
        nread = 2;
    }
    else
    {
        nread = 1;
    }

    // Translate data
    if( nread && dest )
    {
        if( nread == 1 )
        {
            *dest = usrc[0];
        }
        else
        {
            *dest = 0x10000 + ( (( (usrc[0]) & 0x03FF )<<10) | ( (usrc[1]) & 0x03FF ) );
        }
    }

    return nread;
}
//------------------------------------------------------------------------------
INLINE int count_leadingbits(uint8_t byte)
{
    /*
     * 計算一個位元組在二進位資料中從頭起算的連續 1 的數量
     */
    if     ( byte  < 0x80 ) return 0;  // Bin : 0XXXXXXX
    else if( byte  < 0xC0 ) return 1;  // Bin : 10XXXXXX
    else if( byte  < 0xE0 ) return 2;  // Bin : 110XXXXX
    else if( byte  < 0xF0 ) return 3;  // Bin : 1110XXXX
    else if( byte  < 0xF8 ) return 4;  // Bin : 11110XXX
    else if( byte  < 0xFC ) return 5;  // Bin : 111110XX
    else if( byte  < 0xFE ) return 6;  // Bin : 1111110X
    else if( byte == 0xFE ) return 7;  // Bin : 11111110
    else                    return 8;  // Bin : 11111111
}
//------------------------------------------------------------------------------
INLINE int count_u8pairs(const uint8_t* RESTRICT data)
{
    /*
     * 計算資料中每個位元組皆為 10XXXXXX (二進位) 格式的連續排列數量
     */
    int cnt;

    for(( cnt=0 );( ( *data++ & 0xC0 ) == 0x80 );( ++cnt ));

    return cnt;
}
//------------------------------------------------------------------------------
size_t ch_utf8_to_utf32(char32_t* RESTRICT dest, const char* RESTRICT src)
{
    /**
     * @brief Convert UTF-8 data to a single UTF-32 code.
     *
     * @param dest 用來接收轉換結果的變數，可為 NULL。
     * @param src  來源資料。
     * @return 傳回本次轉換中從來源資料陣列所取用的資料數量。
     *
     * @note 在遇到無法正確轉換的資料時，該單一資料位元組將被直接視為一個號碼作為轉換。
     */
    const uint8_t *usrc = (const uint8_t*) src;
    size_t         nread;

    // Calculate the input count
    if( !usrc )
    {
        nread = 0;
    }
    else
    {
        nread = count_leadingbits(usrc[0]);

        if( nread < 2 || 6 < nread )
        {
            nread = 1;
        }
        else
        {
            nread = ( nread > (size_t)count_u8pairs(&usrc[1]) + 1 )?
                    ( 1 ):( nread );
        }
    }

    // Translate data
    if( nread && dest )
    {
        switch( nread )
        {
        case 6:
            *dest = (( usrc[0] & 0x01 )<<30) | (( usrc[1] & 0x3F )<<24) | (( usrc[2] & 0x3F )<<18) | (( usrc[3] & 0x3F )<<12) | (( usrc[4] & 0x3F )<<6) | ( usrc[5] & 0x3F );
            break;
        case 5:
            *dest = (( usrc[0] & 0x03 )<<24) | (( usrc[1] & 0x3F )<<18) | (( usrc[2] & 0x3F )<<12) | (( usrc[3] & 0x3F )<<6) | ( usrc[4] & 0x3F );
            break;
        case 4:
            *dest = (( usrc[0] & 0x07 )<<18) | (( usrc[1] & 0x3F )<<12) | (( usrc[2] & 0x3F )<<6) | ( usrc[3] & 0x3F );
            break;
        case 3:
            *dest = (( usrc[0] & 0x0F )<<12) | (( usrc[1] & 0x3F )<<6) | ( usrc[2] & 0x3F );
            break;
        case 2:
            *dest = (( usrc[0] & 0x1F )<<6) | ( usrc[1] & 0x3F );
            break;
        default:
            *dest = usrc[0];
        }
    }

    return nread;
}
//------------------------------------------------------------------------------
//---- Byte-to-byte data copy --------------------------------------------------
//------------------------------------------------------------------------------
size_t utf8_to_utf8(char* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /**
     * @brief Byte-to-byte data copy.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 byte 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小
     *         (含 null-terminator)，以 byte 為單位。
     *
     * @note 這個函是的目的為建立一個類似 memcpy 的工具。
     *       因為 Wide-Character、Windows TCHAR 等類型在某些狀態下會是一個位元組大小，
     *       因此提供一個界面與本模組函式相似，
     *       但內容僅單純複製資料的函式，供特殊條件下使用。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        while( *src++ ) ++nwrite_total;
        ++nwrite_total;
    }
    else
    {
        while( *src )
        {
            if( !destsz ) return 0;
            *dest++ = *src++;
            ++nwrite_total;
        }
        *dest = 0;
        ++nwrite_total;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
//---- String Translation ( UTF - UTF ) ----------------------------------------
//------------------------------------------------------------------------------
size_t utf32_to_utf16(char16_t* RESTRICT dest, size_t destsz, const char32_t* RESTRICT src)
{
    /**
     * @brief Translate UTF-32 string to UTF-16 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 char16_t 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 char16_t 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            nwrite_total += ch_utf32_to_utf16(NULL, *src++);
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            size_t    nwrite;
            char16_t  ch16[2];
            char16_t *buf;

            // Translate UTF-32 character to UTF-16 format
            nwrite        = ch_utf32_to_utf16(ch16, *src++);
            if( destsz <= nwrite ) return 0;

            // Size calculation
            destsz       -= nwrite;
            nwrite_total += nwrite;

            // Fill data
            for(( buf = ch16 );( nwrite-- );( *dest++ = *buf++ ));
        }
        // Write the null-terminator
        *dest = 0;
        ++nwrite_total;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
size_t utf32_to_utf8(char* RESTRICT dest, size_t destsz, const char32_t* RESTRICT src)
{
    /**
     * @brief Translate UTF-32 string to UTF-8 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 byte 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 byte 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            nwrite_total += ch_utf32_to_utf8(NULL, *src++);
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            size_t nwrite;
            char   ch8[6];
            char  *buf;

            // Translate UTF-32 character to UTF-8 format
            nwrite        = ch_utf32_to_utf8(ch8, *src++);
            if( destsz <= nwrite ) return 0;

            // Size calculation
            destsz       -= nwrite;
            nwrite_total += nwrite;

            // Fill data
            for(( buf = ch8 );( nwrite-- );( *dest++ = *buf++ ));
        }
        // Write the null-terminator
        *dest = 0;
        ++nwrite_total;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
size_t utf16_to_utf32(char32_t* RESTRICT dest, size_t destsz, const char16_t* RESTRICT src)
{
    /**
     * @brief Translate UTF-16 string to UTF-32 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 char32_t 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 char32_t 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            src += ch_utf16_to_utf32(NULL, src);
            ++nwrite_total;
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        size_t destsz0 = destsz;

        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            // Translate UTF-16 character to UTF-32 format
            if( !destsz ) return 0;
            src += ch_utf16_to_utf32(dest++, src);

            // Size calculation
            --destsz;
        }
        // Write the null-terminator
        if( !destsz ) return 0;
        *dest = 0;
        --destsz;
        // Calculate the total write count
        nwrite_total = destsz0 - destsz;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
size_t utf8_to_utf32(char32_t* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /**
     * @brief Translate UTF-8 string to UTF-32 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 char32_t 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 char32_t 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            src += ch_utf8_to_utf32(NULL, src);
            ++nwrite_total;
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        size_t destsz0 = destsz;

        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            // Translate UTF-8 character to UTF-32 format
            if( !destsz ) return 0;
            src += ch_utf8_to_utf32(dest++, src);

            // Size calculation
            --destsz;
        }
        // Write the null-terminator
        if( !destsz ) return 0;
        *dest = 0;
        --destsz;
        // Calculate the total write count
        nwrite_total = destsz0 - destsz;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
size_t utf16_to_utf8(char* RESTRICT dest, size_t destsz, const char16_t* RESTRICT src)
{
    /**
     * @brief Translate UTF-16 string to UTF-8 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 byte 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 byte 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            char32_t ch32;

            src          += ch_utf16_to_utf32(&ch32, src);
            nwrite_total += ch_utf32_to_utf8(NULL, ch32);
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            size_t   nwrite;
            char32_t ch32;
            char     ch8[6];
            char    *buf;

            // Translate UTF-16 character to UTF-32 format
            src   += ch_utf16_to_utf32(&ch32, src);

            // Translate UTF-32 character to UTF-8 format
            nwrite = ch_utf32_to_utf8(ch8, ch32);
            if( destsz <= nwrite ) return 0;

            // Size calculation
            destsz       -= nwrite;
            nwrite_total += nwrite;

            // Fill data
            for(( buf = ch8 );( nwrite-- );( *dest++ = *buf++ ));
        }
        // Write the null-terminator
        *dest = 0;
        ++nwrite_total;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
size_t utf8_to_utf16(char16_t* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /**
     * @brief Translate UTF-8 string to UTF-16 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 char16_t 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 char16_t 為單位。
     */
    size_t nwrite_total = 0;

    if( !src ) return 0;

    if( !dest )
    {
        // 只計算所需的緩衝大小
        while( *src )
        {
            char32_t ch32;

            src          += ch_utf8_to_utf32(&ch32, src);
            nwrite_total += ch_utf32_to_utf16(NULL, ch32);
        }
        ++nwrite_total;  // Don't forget the null-terminator
    }
    else
    {
        // 逐一轉換來源字元，並將結果寫入緩衝空間。
        while( *src )
        {
            size_t    nwrite;
            char32_t  ch32;
            char16_t  ch16[2];
            char16_t *buf;

            // Translate UTF-8 character to UTF-32 format
            src   += ch_utf8_to_utf32(&ch32, src);

            // Translate UTF-32 character to UTF-16 format
            nwrite = ch_utf32_to_utf16(ch16, ch32);
            if( destsz <= nwrite ) return 0;

            // Size calculation
            destsz       -= nwrite;
            nwrite_total += nwrite;

            // Fill data
            for(( buf = ch16 );( nwrite-- );( *dest++ = *buf++ ));
        }
        // Write the null-terminator
        *dest = 0;
        ++nwrite_total;
    }

    return nwrite_total;
}
//------------------------------------------------------------------------------
//---- String Translation ( UTF-8 - Traditional MBS ) --------------------------
//------------------------------------------------------------------------------
size_t utf8_to_mbs(char* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /**
     * @brief Translate UTF-8 string to traditional multi-bytes format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 byte 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 byte 為單位。
     *
     * @note 本函式在第一次被呼叫時會將本行程的 Locale 設定變更為作業系統預設值。
     */
#ifndef NO_WCHAR
    static bool runned      = false;
    wchar_t     buf_static[IGNUTF_INNERBUF_SIZE];
    wchar_t    *buf_dynamic = NULL;
    wchar_t    *buf         = buf_static;
    size_t      wcssz, mbssz;

    // Set locale to system default when first run
    if( !runned )
    {
        runned = true;
#ifdef WINCE
        // Windows CE did not support "setlocale"
#else
        setlocale(LC_ALL, "");
#endif
    }

    // Calculate buffer size needed
    wcssz = utf8_to_wcs(NULL, 0, src);
    if( !wcssz ) return 0;
    // Buffer allocate if needed
    if( wcssz > IGNUTF_INNERBUF_SIZE )
    {
        buf = buf_dynamic = malloc( wcssz * sizeof(wchar_t) );
        if( !buf ) return 0;
    }

    // Translate to wide-character format
    utf8_to_wcs(buf, wcssz, src);
    // Calculate destination size needed
    mbssz = wcstombs(NULL, buf, 0) + 1;

    // Translate to multi-bytes format
    if( !dest )
    {
        // Nothing to do
    }
    else if( destsz < mbssz )
    {
        mbssz = 0;
    }
    else
    {
        wcstombs(dest, buf, mbssz);
    }

    // Free the buffer if we allocated it
    if( buf_dynamic )
    {
        free(buf_dynamic);
    }

    return mbssz;
#else
    return utf8_to_utf8(dest, destsz, src);
#endif // NO_WCHAR
}
//------------------------------------------------------------------------------
size_t mbs_to_utf8(char* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /**
     * @brief Translate traditional multi-bytes string to UTF-8 format.
     *
     * @param dest   用來接收轉換結果的緩衝區，可為 NULL。
     * @param destsz 緩衝區大小，以 byte 為單位。
     * @param src    來源字串。
     * @return 轉換成功傳回寫入緩衝區的陣列元素數量(含 null-terminator)；
     *         轉換失敗傳回零；
     *         若 dest 為 NULL 則傳回 dest 緩衝區所需的最小大小(含 null-terminator)，
     *         以 byte 為單位。
     *
     * @note 本函式在第一次被呼叫時會將本行程的 Locale 設定變更為作業系統預設值。
     */
#ifndef NO_WCHAR
    static bool runned      = false;
    wchar_t     buf_static[IGNUTF_INNERBUF_SIZE];
    wchar_t    *buf_dynamic = NULL;
    wchar_t    *buf         = buf_static;
    size_t      wcssz, u8sz;

    // Set locale to system default when first run
    if( !runned )
    {
        runned = true;
#ifdef WINCE
        // Windows CE did not support "setlocale"
#else
        setlocale(LC_ALL, "");
#endif
    }

    // Calculate buffer size needed
    wcssz = mbstowcs(NULL, src, 0) + 1;
    if( !wcssz ) return 0;
    // Buffer allocate if needed
    if( wcssz > IGNUTF_INNERBUF_SIZE )
    {
        buf = buf_dynamic = malloc( wcssz * sizeof(wchar_t) );
        if( !buf ) return 0;
    }

    // Translate to wide-character format
    mbstowcs(buf, src, wcssz);
    // Calculate destination size needed
    u8sz = wcs_to_utf8(NULL, 0, buf);

    // Translate to UTF-8 format
    if( !dest )
    {
        // Nothing to do
    }
    else if( destsz < u8sz )
    {
        u8sz = 0;
    }
    else
    {
        wcs_to_utf8(dest, u8sz, buf);
    }

    // Free the buffer if we allocated it
    if( buf_dynamic )
    {
        free(buf_dynamic);
    }

    return u8sz;
#else
    return utf8_to_utf8(dest, destsz, src);
#endif // NO_WCHAR
}
//------------------------------------------------------------------------------
//---- String format identify --------------------------------------------------
//------------------------------------------------------------------------------
bool is_utf8_encoding(const void* RESTRICT buf, size_t size)
{
    /**
     * @brief 檢查一段資料或字串是否為 UTF-8 格式。
     *
     * @param buf  傳入欲做檢查的資料或字串。
     * @param size 傳入該筆資料的大小。
     * @return 當傳入資料被判別為 UTF-8 格式時傳回 true；否則傳回 false。
     */
    uint8_t *dat         = (uint8_t*)buf;
    size_t   nread;
    size_t   nread_total = 0;
    size_t   nerror      = 0;

    if( !dat || !size ) return false;

    while( size )
    {
        size_t nbits = count_leadingbits(*dat);

        switch( nbits )
        {
        case 0:
            nread = 1;
            break;

        case 1:
        case 7:
        case 8:
            nread = 1;
            ++nerror;
            break;

        default:
            nread = ( nbits == 1 + count_u8pairs(dat+1) )?
                    ( nbits ):( 1 );
            if( nread == 1 ) ++nerror;

        }

        if( nread > size ) return false;

        nread_total += nread;
        dat         += nread;
        size        -= nread;
    }

    return !nerror;
}
//------------------------------------------------------------------------------

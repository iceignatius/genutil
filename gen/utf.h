/**
 * @file
 * @brief     UTF translator
 * @details   Translate strings between different Unicode UTF format.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_UTF_H_
#define _GEN_UTF_H_

#ifdef __cplusplus
#include <string>
#endif

#include "type.h"
#include "inline.h"
#include "restrict.h"

#define IGNUTF_INNERBUF_SIZE 512  // 內部為加速運算而使用的緩衝區大小，此數值可依需求調整。

#ifdef __cplusplus
extern "C" {
#endif

// Type definition

#ifndef _TCHAR_DEFINED
    #define IGNUTF_TCHAR_DEFINED
    #if defined(_WIN32) && defined(UNICODE)
        #define TCHAR wchar_t
    #else
        #define TCHAR char
    #endif
#endif

/**
 * @name Character translation
 * @{
 */

size_t ch_utf32_to_utf16(char16_t dest[2], char32_t src);
size_t ch_utf32_to_utf8 (char     dest[6], char32_t src);

size_t ch_utf16_to_utf32(char32_t* RESTRICT dest, const char16_t* RESTRICT src);
size_t ch_utf8_to_utf32 (char32_t* RESTRICT dest, const char    * RESTRICT src);

/**
 * @}
 */

/**
 * @file
 * @note 基於效率的考量，所有字串轉換函式不會先檢查傳入的緩衝區大小是否足夠。
 *       除了無緩衝區傳入的狀況外，函式會直接開始轉換程序，
 *       在轉換過程中發現緩衝空間不足時才會傳回轉換失敗的結果。
 *       亦即無論資料是否轉換成功，緩衝區都會被寫入資料。
 */

/**
 * @name Byte-to-byte data copy
 * @{
 */

size_t utf8_to_utf8(char* RESTRICT dest, size_t destsz, const char* RESTRICT src);

/**
 * @}
 */

/**
 * @name String translation ( UTF - UTF )
 * @{
 */

size_t utf32_to_utf16(char16_t* RESTRICT dest, size_t destsz, const char32_t* RESTRICT src);
size_t utf32_to_utf8 (char    * RESTRICT dest, size_t destsz, const char32_t* RESTRICT src);

size_t utf16_to_utf32(char32_t* RESTRICT dest, size_t destsz, const char16_t* RESTRICT src);
size_t utf8_to_utf32 (char32_t* RESTRICT dest, size_t destsz, const char    * RESTRICT src);

size_t utf16_to_utf8 (char    * RESTRICT dest, size_t destsz, const char16_t* RESTRICT src);
size_t utf8_to_utf16 (char16_t* RESTRICT dest, size_t destsz, const char    * RESTRICT src);

/**
 * @}
 */

#ifndef NO_WCHAR

/**
 * @name String translation ( UTF-8 - WCS )
 * @{
 */

INLINE size_t utf8_to_wcs(wchar_t* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /// Translate UTF-8 string to wide-character format.
#if   IGNDEF_SIZEOF_WCHAR == 4
    return utf8_to_utf32((char32_t*)dest, destsz, src);
#elif IGNDEF_SIZEOF_WCHAR == 2
    return utf8_to_utf16((char16_t*)dest, destsz, src);
#elif IGNDEF_SIZEOF_WCHAR == 1
    return utf8_to_utf8 ((char*    )dest, destsz, src);
#else
    #error No implementation on this platform!
#endif
}

INLINE size_t wcs_to_utf8(char* RESTRICT dest, size_t destsz, const wchar_t* RESTRICT src)
{
    /// Translate wide-character string to UTF-8 format.
#if   IGNDEF_SIZEOF_WCHAR == 4
    return utf32_to_utf8(dest, destsz, (const char32_t*)src);
#elif IGNDEF_SIZEOF_WCHAR == 2
    return utf16_to_utf8(dest, destsz, (const char16_t*)src);
#elif IGNDEF_SIZEOF_WCHAR == 1
    return utf8_to_utf8 (dest, destsz, (const char*    )src);
#else
    #error No implementation on this platform!
#endif
}

/**
 * @}
 */

#endif // NO_WCHAR

/**
 * @name String translation ( UTF-8 - Traditional MBS )
 * @{
 */

size_t utf8_to_mbs(char* RESTRICT dest, size_t destsz, const char* RESTRICT src);
size_t mbs_to_utf8(char* RESTRICT dest, size_t destsz, const char* RESTRICT src);

/**
 * @}
 */

/**
 * @name String translation ( UTF-8 - WINCHAR )
 * @{
 */

INLINE size_t utf8_to_winstr(TCHAR* RESTRICT dest, size_t destsz, const char* RESTRICT src)
{
    /// Translate UTF-8 string to Windows TCHAR format.
#if   defined(_WIN32) &&  defined(UNICODE) && !defined(NO_WCHAR)
    return utf8_to_wcs (dest, destsz, src);
#elif defined(_WIN32) && !defined(UNICODE) && !defined(NO_WCHAR)
    return utf8_to_mbs (dest, destsz, src);
#else
    return utf8_to_utf8(dest, destsz, src);
#endif
}

INLINE size_t winstr_to_utf8(char* RESTRICT dest, size_t destsz, const TCHAR* RESTRICT src)
{
    /// Translate Windows TCHAR string to UTF-8 format.
#if   defined(_WIN32) &&  defined(UNICODE) && !defined(NO_WCHAR)
    return wcs_to_utf8 (dest, destsz, src);
#elif defined(_WIN32) && !defined(UNICODE) && !defined(NO_WCHAR)
    return mbs_to_utf8 (dest, destsz, src);
#else
    return utf8_to_utf8(dest, destsz, src);
#endif
}

/**
 * @}
 */

/**
 * @name String format identify
 * @{
 */

bool is_utf8_encoding(const void* RESTRICT buf, size_t size);

/**
 * @}
 */

/**
 * @{
 */
#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

#if __cplusplus < 201103
namespace std
{
    typedef basic_string<char16_t> u16string;  ///< The same as @a std::u16string in C++11.
    typedef basic_string<char32_t> u32string;  ///< The same as @a std::u32string in C++11.
}
#endif

/// String object for Windows CHAR character type.
#if defined(_WIN32) && defined(UNICODE)
    typedef std::wstring win_string;
#else
    typedef std::string  win_string;
#endif

#endif  // __cplusplus
/**
 * @}
 */

/**
 * @name C++ wrapper
 * @{
 */
#ifdef __cplusplus

inline std::u16string Utf32ToUtf16(const std::u32string &str)
{
    /// @see ::utf32_to_utf16
    std::u16string res;
    size_t         size = utf32_to_utf16(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf32_to_utf16((char16_t*)res.c_str(), size, str.c_str());

    return res;
}

inline std::string Utf32ToUtf8(const std::u32string &str)
{
    /// @see ::utf32_to_utf8
    std::string res;
    size_t      size = utf32_to_utf8(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf32_to_utf8((char*)res.c_str(), size, str.c_str());

    return res;
}

inline std::u32string Utf16ToUtf32(const std::u16string &str)
{
    /// @see ::utf16_to_utf32
    std::u32string res;
    size_t         size = utf16_to_utf32(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf16_to_utf32((char32_t*)res.c_str(), size, str.c_str());

    return res;
}

inline std::u32string Utf8ToUtf32(const std::string &str)
{
    /// @see ::utf8_to_utf32
    std::u32string res;
    size_t         size = utf8_to_utf32(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf8_to_utf32((char32_t*)res.c_str(), size, str.c_str());

    return res;
}

inline std::string Utf16ToUtf8(const std::u16string &str)
{
    /// @see ::utf16_to_utf8
    std::string res;
    size_t      size = utf16_to_utf8(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf16_to_utf8((char*)res.c_str(), size, str.c_str());

    return res;
}

inline std::u16string Utf8ToUtf16(const std::string &str)
{
    /// @see ::utf8_to_utf16
    std::u16string res;
    size_t         size = utf8_to_utf16(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf8_to_utf16((char16_t*)res.c_str(), size, str.c_str());

    return res;
}

#ifndef NO_WCHAR
inline std::wstring Utf8ToWcs(const std::string &str)
{
    /// @see ::utf8_to_wcs
    std::wstring res;
    size_t       size = utf8_to_wcs(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf8_to_wcs((wchar_t*)res.c_str(), size, str.c_str());

    return res;
}
#endif // NO_WCHAR

#ifndef NO_WCHAR
inline std::string WcsToUtf8(const std::wstring &str)
{
    /// @see ::wcs_to_utf8
    std::string res;
    size_t      size = wcs_to_utf8(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    wcs_to_utf8((char*)res.c_str(), size, str.c_str());

    return res;
}
#endif // NO_WCHAR

inline std::string Utf8ToMbs(const std::string &str)
{
    /// @see ::utf8_to_mbs
    std::string res;
    size_t      size = utf8_to_mbs(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf8_to_mbs((char*)res.c_str(), size, str.c_str());

    return res;
}

inline std::string MbsToUtf8(const std::string &str)
{
    /// @see ::mbs_to_utf8
    std::string res;
    size_t      size = mbs_to_utf8(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    mbs_to_utf8((char*)res.c_str(), size, str.c_str());

    return res;
}

inline win_string Utf8ToWinstr(const std::string &str)
{
    /// @see ::utf8_to_winstr
    win_string res;
    size_t     size = utf8_to_winstr(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    utf8_to_winstr((TCHAR*)res.c_str(), size, str.c_str());

    return res;
}

inline std::string WinstrToUtf8(const win_string &str)
{
    /// @see ::winstr_to_utf8
    std::string res;
    size_t      size = winstr_to_utf8(NULL, 0, str.c_str());

    res.resize( size ? size-1 : 0 );
    winstr_to_utf8((char*)res.c_str(), size, str.c_str());

    return res;
}

inline bool IsUtf8Encoding(const std::string &str)
{
    /// @see ::is_utf8_encoding
    return is_utf8_encoding(str.c_str(), str.size());
}

#endif  // C++ wrapper
/**
 * @}
 */

#ifdef IGNUTF_TCHAR_DEFINED
    #undef TCHAR
    #undef IGNUTF_TCHAR_DEFINED
#endif

#endif

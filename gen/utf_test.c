/*
 * UTF 工具測試程式
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "utf.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

//--------------------------------------
//---- Test Samples (character) --------
//--------------------------------------

// * U+55
static const char32_t sample01_32   = 0x55;
static const char16_t sample01_16[] = {0x0055};
static const char     sample01_8 [] = {0x55};
// * U+7E
static const char32_t sample02_32   = 0x7E;
static const char16_t sample02_16[] = {0x007E};
static const char     sample02_8 [] = {0x7E};
// * U+53A
static const char32_t sample03_32   = 0x053A;
static const char16_t sample03_16[] = {0x053A};
static const char     sample03_8 [] = {0xD4,0xBA};
// * U+7BC
static const char32_t sample04_32   = 0x07BC;
static const char16_t sample04_16[] = {0x07BC};
static const char     sample04_8 [] = {0xDE,0xBC};
// * U+9A8C
static const char32_t sample05_32   = 0x9A8C;
static const char16_t sample05_16[] = {0x9A8C};
static const char     sample05_8 [] = {0xE9,0xAA,0x8C};
// * U+2391
static const char32_t sample06_32   = 0x2391;
static const char16_t sample06_16[] = {0x2391};
static const char     sample06_8 [] = {0xE2,0x8E,0x91};
// * U+5DBC9
static const char32_t sample07_32   = 0x5DBC9;
static const char16_t sample07_16[] = {0xD936,0xDFC9};
static const char     sample07_8 [] = {0xF1,0x9D,0xAF,0x89};
// * U+C9596
static const char32_t sample08_32   = 0xC9596;
static const char16_t sample08_16[] = {0xDAE5,0xDD96};
static const char     sample08_8 [] = {0xF3,0x89,0x96,0x96};
// * U+27C2F77
static const char32_t sample09_32   = 0x027C2F77;
static const char     sample09_8 [] = {0xFA,0x9F,0x82,0xBD,0xB7};
// * U+1C8A0D0
static const char32_t sample10_32   = 0x01C8A0D0;
static const char     sample10_8 [] = {0xF9,0xB2,0x8A,0x83,0x90};
// * U+63ACAF58
static const char32_t sample11_32   = 0x63ACAF58;
static const char     sample11_8 [] = {0xFD,0xA3,0xAB,0x8A,0xBD,0x98};
// * U+34345068
static const char32_t sample12_32   = 0x34345068;
static const char     sample12_8 [] = {0xFC,0xB4,0x8D,0x85,0x81,0xA8};

//--------------------------------------
//---- Test Samples (string) -----------
//--------------------------------------

// * UTF-32 string
static const char32_t string_32[] =
{
    0x55,
    0x7E,
    0x053A,
    0x07BC,
    0x9A8C,
    0x2391,
    0x5DBC9,
    0xC9596,
    0x00
};
static const size_t strcnt_32 = sizeof(string_32)/sizeof(string_32[0]);

// * UTF-16 string
static const char16_t string_16[] =
{
    0x0055,
    0x007E,
    0x053A,
    0x07BC,
    0x9A8C,
    0x2391,
    0xD936,0xDFC9,
    0xDAE5,0xDD96,
    0x0000
};
static const size_t strcnt_16 = sizeof(string_16)/sizeof(string_16[0]);

// * UTF-8 string
static const char string_8[] =
{
    0x55,
    0x7E,
    0xD4,0xBA,
    0xDE,0xBC,
    0xE9,0xAA,0x8C,
    0xE2,0x8E,0x91,
    0xF1,0x9D,0xAF,0x89,
    0xF3,0x89,0x96,0x96,
    0x00
};
static const size_t strcnt_8 = sizeof(string_8)/sizeof(string_8[0]);

//--------------------------------------
//---- Main Test Process ---------------
//--------------------------------------

#define UTFTEST_BUFCNT 1024

int main(int argc, char *argv[])
{

    // Charactor translation test : UTF-32 and UTF-16
    {
        char16_t buf[2];
        size_t   cnt, cnt_target;

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample01_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample01_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample02_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample02_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample03_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample03_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample04_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample04_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample05_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample05_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample06_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample06_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 2;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample07_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample07_16, cnt_target*sizeof(char16_t)) );

        cnt_target = 2;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf16(buf, sample08_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample08_16, cnt_target*sizeof(char16_t)) );
    }

    // Charactor translation test : UTF-32 and UTF-8
    {
        char buf[6];
        int  cnt, cnt_target;

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample01_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample01_8, cnt_target) );

        cnt_target = 1;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample02_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample02_8, cnt_target) );

        cnt_target = 2;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample03_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample03_8, cnt_target) );

        cnt_target = 2;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample04_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample04_8, cnt_target) );

        cnt_target = 3;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample05_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample05_8, cnt_target) );

        cnt_target = 3;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample06_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample06_8, cnt_target) );

        cnt_target = 4;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample07_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample07_8, cnt_target) );

        cnt_target = 4;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample08_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample08_8, cnt_target) );

        cnt_target = 5;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample09_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample09_8, cnt_target) );

        cnt_target = 5;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample10_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample10_8, cnt_target) );

        cnt_target = 6;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample11_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample11_8, cnt_target) );

        cnt_target = 6;
        memset(buf, 0, sizeof(buf));
        cnt = ch_utf32_to_utf8(buf, sample12_32);
        assert( cnt == cnt_target );
        assert( 0 == memcmp(buf, sample12_8, cnt_target) );
    }

    // Charactor translation test : UTF-16 and UTF-32
    {
        int      cnt, cnt_target;
        char32_t code;
        char16_t invalid_utf16[] = {0xDC00,0xD800};

        assert( 1 == ch_utf16_to_utf32(&code, invalid_utf16) );
        assert( code == (uint16_t)invalid_utf16[0] );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample01_16);
        assert( cnt  == cnt_target );
        assert( code == sample01_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample02_16);
        assert( cnt  == cnt_target );
        assert( code == sample02_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample03_16);
        assert( cnt == cnt_target );
        assert( code  == sample03_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample04_16);
        assert( cnt  == cnt_target );
        assert( code == sample04_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample05_16);
        assert( cnt  == cnt_target );
        assert( code == sample05_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample06_16);
        assert( cnt  == cnt_target );
        assert( code == sample06_32 );

        cnt_target = 2;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample07_16);
        assert( cnt  == cnt_target );
        assert( code == sample07_32 );

        cnt_target = 2;
        code       = 0;
        cnt = ch_utf16_to_utf32(&code, sample08_16);
        assert( cnt  == cnt_target );
        assert( code == sample08_32 );
    }

    // Charactor translation test : UTF-8 and UTF-32
    {
        int      cnt, cnt_target;
        char32_t code;

        assert( 1 == ch_utf8_to_utf32(&code, "\xFF\xFF") );
        assert( code == 0xFF );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample01_8);
        assert( cnt  == cnt_target );
        assert( code == sample01_32 );

        cnt_target = 1;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample02_8);
        assert( cnt  == cnt_target );
        assert( code == sample02_32 );

        cnt_target = 2;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample03_8);
        assert( cnt  == cnt_target );
        assert( code == sample03_32 );

        cnt_target = 2;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample04_8);
        assert( cnt  == cnt_target );
        assert( code ==sample04_32 );

        cnt_target = 3;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample05_8);
        assert( cnt  == cnt_target );
        assert( code == sample05_32 );

        cnt_target = 3;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample06_8);
        assert( cnt  == cnt_target );
        assert( code == sample06_32 );

        cnt_target = 4;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample07_8);
        assert( cnt  == cnt_target );
        assert( code == sample07_32 );

        cnt_target = 4;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample08_8);
        assert( cnt  == cnt_target );
        assert( code == sample08_32 );

        cnt_target = 5;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample09_8);
        assert( cnt  == cnt_target );
        assert( code == sample09_32 );

        cnt_target = 5;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample10_8);
        assert( cnt  == cnt_target );
        assert( code == sample10_32 );

        cnt_target = 6;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample11_8);
        assert( cnt  == cnt_target );
        assert( code == sample11_32 );

        cnt_target = 6;
        code       = 0;
        cnt = ch_utf8_to_utf32(&code, sample12_8);
        assert( cnt  == cnt_target );
        assert( code == sample12_32 );
    }

    // Byte-to-byte data copy test
    {
        const char data0[] = "Byte-to-byte data copy test";
        char       data1[1024];
        size_t     data1sz;

        data1sz = utf8_to_utf8(NULL, 0, data0);
        assert( data1sz == sizeof(data0) );

        data1sz = utf8_to_utf8(data1, sizeof(data1), data0);
        assert( data1sz == sizeof(data0) );
        assert( 0 == memcmp(data1, data0, sizeof(data0)) );
    }

    // String translation test : UTF-32 and UTF-16
    {
        char16_t buf[UTFTEST_BUFCNT];
        size_t   nwrite;

        assert( strcnt_16 == utf32_to_utf16(NULL, 0, string_32) );
        nwrite = utf32_to_utf16(buf, UTFTEST_BUFCNT, string_32);
        assert( nwrite == strcnt_16 );
        assert( 0 == memcmp(buf, string_16, nwrite*sizeof(char16_t)) );
    }
    // String translation test : UTF-32 and UTF-8
    {
        char   buf[UTFTEST_BUFCNT];
        size_t nwrite;

        assert( strcnt_8 == utf32_to_utf8(NULL, 0, string_32) );
        nwrite = utf32_to_utf8(buf, UTFTEST_BUFCNT, string_32);
        assert( nwrite == strcnt_8 );
        assert( 0 == memcmp(buf, string_8, nwrite*sizeof(char)) );
    }

    // String translation test : UTF-16 and UTF-32
    {
        char32_t buf[UTFTEST_BUFCNT];
        size_t   nwrite;

        assert( strcnt_32 == utf16_to_utf32(NULL, 0, string_16) );
        nwrite = utf16_to_utf32(buf, UTFTEST_BUFCNT, string_16);
        assert( nwrite == strcnt_32 );
        assert( 0 == memcmp(buf, string_32, nwrite*sizeof(char32_t)) );
    }
    // String translation test : UTF-8 and UTF-32
    {
        char32_t buf[UTFTEST_BUFCNT];
        size_t   nwrite;

        assert( strcnt_32 == utf8_to_utf32(NULL, 0, string_8) );
        nwrite = utf8_to_utf32(buf, UTFTEST_BUFCNT, string_8);
        assert( nwrite == strcnt_32 );
        assert( 0 == memcmp(buf, string_32, nwrite*sizeof(char32_t)) );
    }

    // String translation test : UTF-16 and UTF-8
    {
        char   buf[UTFTEST_BUFCNT];
        size_t nwrite;

        assert( strcnt_8 == utf16_to_utf8(NULL, 0, string_16) );
        nwrite = utf16_to_utf8(buf, UTFTEST_BUFCNT, string_16);
        assert( nwrite == strcnt_8 );
        assert( 0 == memcmp(buf, string_8, nwrite*sizeof(char)) );
    }
    // String translation test : UTF-8 and UTF-16
    {
        char16_t buf[UTFTEST_BUFCNT];
        size_t   nwrite;

        assert( strcnt_16 == utf8_to_utf16(NULL, 0, string_8) );
        nwrite = utf8_to_utf16(buf, UTFTEST_BUFCNT, string_8);
        assert( nwrite == strcnt_16 );
        assert( 0 == memcmp(buf, string_16, nwrite*sizeof(char16_t)) );
    }

#ifndef NO_WCHAR
    // String translation test : UTF-8 and wide-character
    {
        const char str0[] = "Wide-character string translation test.";
        wchar_t    wcs[1024];
        size_t     wcssz;
        char       str1[1024];
        size_t     str1sz;

        wcssz  = utf8_to_wcs(wcs, sizeof(wcs)/sizeof(wcs[0]), str0);
        str1sz = wcs_to_utf8(str1  , sizeof(str1)  , wcs);

        printf("UTF-8 and WCS test\n");
        printf("  string 0   : %d [%s]\n" , (int)sizeof(str0), str0);
        printf("  string WCS : %d [%ls]\n", (int)wcssz       , wcs);
        printf("  string 1   : %d [%s]\n" , (int)str1sz      , str1);

        assert( wcssz == sizeof(str0) );

        assert( str1sz == sizeof(str0) );
        assert( 0 == memcmp(str1, str0, sizeof(str0)) );
    }
#endif // NO_WCHAR

    // String translation test : UTF-8 and traditional multi-bytes
    {
        const char str0[] = "Traditional multi-bytes string translation test.";
        char       strmul[1024];
        size_t     strmulsz;
        char       str1[1024];
        size_t     str1sz;

        strmulsz = utf8_to_mbs(strmul, sizeof(strmul), str0);
        str1sz   = mbs_to_utf8(str1  , sizeof(str1)  , strmul);

        printf("UTF-8 and MBS test\n");
        printf("  string 0   : %d [%s]\n", (int)sizeof(str0), str0);
        printf("  string MBS : %d [%s]\n", (int)strmulsz    , strmul);
        printf("  string 1   : %d [%s]\n", (int)str1sz      , str1);

        assert( strmulsz == sizeof(str0) );
        assert( 0 == memcmp(strmul, str0, sizeof(str0)) );

        assert( str1sz == sizeof(str0) );
        assert( 0 == memcmp(str1, str0, sizeof(str0)) );
    }

    // String format identify
    {
        const char str_utf8[] =
        {
            0xE6, 0x88, 0x91, 0xE6, 0x98, 0xAF, 0xE4, 0xB9, 0x8B, 0xE5, 0xB0, 0x8F,
            0xE5, 0xB0, 0x8F, 0xE9, 0xB3, 0xA5, 0xEF, 0xBC, 0x8C, 0xE9, 0xA3, 0x9B,
            0xE5, 0xB0, 0xB1, 0xE9, 0xA3, 0x9B, 0xE3, 0x80, 0x81, 0xE5, 0x8F, 0xAB,
            0xE5, 0xB0, 0xB1, 0xE5, 0x8F, 0xAB, 0xEF, 0xBC, 0x8C, 0xE8, 0x87, 0xAA,
            0xE7, 0x94, 0xB1, 0xE9, 0x80, 0x8D, 0xE9, 0x81, 0x99, 0xE3, 0x80, 0x82
        };
        const char str_big5[] =
        {
            0xA7, 0xDA, 0xAC, 0x4F, 0xA4, 0xA7, 0xA4, 0x70,
            0xA4, 0x70, 0xB3, 0xBE, 0xA1, 0x41, 0xAD, 0xB8,
            0xB4, 0x4E, 0xAD, 0xB8, 0xA1, 0x42, 0xA5, 0x73,
            0xB4, 0x4E, 0xA5, 0x73, 0xA1, 0x41, 0xA6, 0xDB,
            0xA5, 0xD1, 0xB3, 0x70, 0xBB, 0xBB, 0xA1, 0x43
        };

        assert(  is_utf8_encoding(str_utf8, sizeof(str_utf8)) );
        assert( !is_utf8_encoding(str_big5, sizeof(str_big5)) );
    }

    return 0;
}

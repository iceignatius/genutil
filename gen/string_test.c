#include <assert.h>
#include <string.h>
#include "string.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

void test_trim_space(void)
{
    {
        char       str[] = "  aa  bb  cc  ";
        const char res[] = "  aa  bb  cc";

        assert( 0 == strcmp(res, str_trim_trailing_space(str)) );
    }

    {
        char       str[] = "aa  bb  cc";
        const char res[] = "aa  bb  cc";

        assert( 0 == strcmp(res, str_trim_trailing_space(str)) );
    }

    {
        char       str[] = "          ";
        const char res[] = "";

        assert( 0 == strcmp(res, str_trim_trailing_space(str)) );
    }

    {
        char       str[] = "";
        const char res[] = "";

        assert( 0 == strcmp(res, str_trim_trailing_space(str)) );
    }

    {
        char       str[] = "  aa  bb  cc  ";
        const char res[] = "aa  bb  cc  ";

        assert( 0 == strcmp(res, str_trim_leading_space(str)) );
    }

    {
        char       str[] = "aa  bb  cc";
        const char res[] = "aa  bb  cc";

        assert( 0 == strcmp(res, str_trim_leading_space(str)) );
    }

    {
        char       str[] = "          ";
        const char res[] = "";

        assert( 0 == strcmp(res, str_trim_leading_space(str)) );
    }

    {
        char       str[] = "";
        const char res[] = "";

        assert( 0 == strcmp(res, str_trim_leading_space(str)) );
    }
}

void test_escch(void)
{
    {
        char       str[] = "ab \\a cd \\b ef \\t gh \\n ij \\v kl \\f mn \\r op \\\\ "
                           "qr \\\" st \\' uv \\? wx \\x19 zy";
        const char res[] = "ab \a cd \b ef \t gh \n ij \v kl \f mn \r op \\ "
                           "qr \" st \' uv \? wx \x19 zy";

        assert( str_escch_to_ctrlcode(str) );
        assert( 0 == strcmp(str, res) );
    }

    {
        char envalid1[] = "ab \\s cd";
        char envalid2[] = "ab\\";
        char envalid3[] = "ab\\xyz";
        char envalid4[] = "ab\\x";

        assert( !str_escch_to_ctrlcode(envalid1) );
        assert( !str_escch_to_ctrlcode(envalid2) );
        assert( !str_escch_to_ctrlcode(envalid3) );
        assert( !str_escch_to_ctrlcode(envalid4) );
    }
}

void test_str_paste(void)
{
    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 0;
        const char *res     = "abcdefgh90123456";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 4;
        const char *res     = "1234abcdefgh3456";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 8;
        const char *res     = "12345678abcdefgh";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 12;
        const char *res     = "123456789012abcdefgh";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 20;
        const char *res     = "1234567890123456    abcdefgh";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 28;
        const char *res     = "1234567890123456            abcd";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }

    {
        char        buf[32] = "1234567890123456\x00mmmmmmmmmmmmmm";
        char        src[]   = "abcdefgh";
        size_t      off     = 36;
        const char *res     = "1234567890123456                ";

        assert( str_paste(buf, sizeof(buf), src, off) );
        assert( 0 == strncmp(buf, res, sizeof(buf)) );
    }
}

void test_hexstr(void)
{
    // Binary to string test - normal.
    {
        const char dat[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
        const char res[] = "123456789ABC";
        char       buf[64];

        assert( ( strlen(res) + 1 ) == str_bin_to_hexstr(buf, sizeof(buf), dat, sizeof(dat), NULL, NULL) );
        assert( 0 == strcmp(buf, res) );

        assert( 0 == str_bin_to_hexstr(buf, 5, dat, sizeof(dat), NULL, NULL) );  // Buffer too small
    }

    // Binary to string test - have decoration.
    {
        const char dat[]     = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
        const char prefix[]  = "<0x";
        const char postfix[] = ">,";
        const char res[]     = "<0x12>,<0x34>,<0x56>,<0x78>,<0x9A>,<0xBC>,";
        char       buf[64];

        assert( ( strlen(res) + 1 ) == str_bin_to_hexstr(buf, sizeof(buf), dat, sizeof(dat), prefix, postfix) );
        assert( 0 == strcmp(buf, res) );

        assert( 0 == str_bin_to_hexstr(buf, 5, dat, sizeof(dat), prefix, postfix) );  // Buffer too small
    }

    // String to binary - normal.
    {
        const char str[] = "123456789ABC";
        const char res[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
        char       buf[32];

        assert( sizeof(res) == str_hexstr_to_bin(buf, sizeof(buf), str, false) );
        assert( 0 == memcmp(buf, res, sizeof(res)) );

        assert( 0 == str_hexstr_to_bin(buf, 5, str, false) );  // Buffer too small
    }

    // String to binary - have noise.
    {
        const char str[] = "--1234-5678-9ABC--";
        const char res[] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
        char       buf[32];

        assert( 0 == str_hexstr_to_bin(buf, sizeof(buf), str  , false) );  // Error by noise
        assert( 0 == str_hexstr_to_bin(buf, sizeof(buf), "123", false) );  // Error by lonely character

        assert( sizeof(res) == str_hexstr_to_bin(buf, sizeof(buf), str, true) );  // Force skip unreadable
        assert( 0 == memcmp(buf, res, sizeof(res)) );
    }
}

int main(void)
{
    test_trim_space();
    test_escch();
    test_str_paste();
    test_hexstr();

    return 0;
}

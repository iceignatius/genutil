#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../mtrand.h"
#include "./const.h"
#include "./base.h"
#include "./io.h"
#include "./math.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#if BIGNUM_DIGSZ != 4
    #error These test cases must run with BIGNUM_DIGSZ equal to 4!
#endif

/*
 * Test Sample
 */

static const uint32_t aa[] =
{
    0xFF,
    0x0228EC,

    0x7B472132,
    0x55C09EA9,
    0x72681692,
    0x5CA16E79,
    0x153AB09F,
    0x24FC,
    0x0551,
    0x6983,
    0x2019,
    0x00,
    0x27F6C14B,
    0x65B27E93,
    0x7E66AC86,
    0x594E2408,
    0x5BE08DD4,
    0x0250,
    0x3246,
    0x6E5D,
    0x6B0F,
    0x00
};

static const uint32_t bb[] =
{
    0xFF,
    0x0115,

    0x0E152A59,
    0x39E9A519,
    0x694C48B4,
    0x50DA3F14,
    0x32010F28,
    0x23945EA8,
    0x10A1214A,
    0x14DB045A,
    0x5B27C234,
    0x576825E5,
    0x1913,
    0x6656,
    0x26CD,
    0x0B7B,
    0x00,
    0x3C08,
    0x3372,
    0x28AA,
    0x3A6E,
    0x00
};

static const uint32_t ans_of_power[] =
{
    0x7FFEFF,
    0x00,

    0x00,
    0x30325169,
    0x00,
    0x334830E1,
    0xDC11B701,
    0x00,
    0x39133E21,
    0x23ADB9E9,
    0x14AB2461,
    0x00,
    0xC9E4D733,
    0xD8255B39,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
    0x72311BE9,
    0xDFE52A21,
    0x01
};

static const uint32_t ans_of_powermod[] =
{
    0xC3,
    0x86,
    0x0277,
    0x31,
    0x07,
    0x0154,
    0x0207,
    0x0306,
    0x02DA,
    0x01,
    0x019E,
    0x00,
    0x0107,
    0x7F,
    0x02DB,
    0x0280,
    0,       // No use
    0x25,
    0x24,
    0x40,
    0x01,
    0        // No use
};

static const uint32_t gcd_of_aa_and_bb[] =
{
    0xFF,
    0x01,

    0x01,
    0x01,
    0x02,
    0x01,
    0x01,
    0x04,
    0x01,
    0x01,
    0x01,
    0x576825E5,
    0x01,
    0x01,
    0x03,
    0x01,
    0x5BE08DD4,
    0x08,
    0x1E,
    0x01,
    0x01,
    0x00
};

static const int testcnt = sizeof(aa)/sizeof(aa[0]);

static const int prime_table[] =
{
    // 百位數的質數列表
    101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
    151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
    199, 211, 223, 227, 229, 233, 239, 241, 251, 257,
    263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
    317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
    383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
    443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
    503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
    577, 587, 593, 599, 601, 607, 613, 617, 619, 631,
    641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
    701, 709, 719, 727, 733, 739, 743, 751, 757, 761,
    769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
    839, 853, 857, 859, 863, 877, 881, 883, 887, 907,
    911, 919, 929, 937, 941, 947, 953, 967, 971, 977,
    983, 991, 997
};

static const int prime_table_cnt = sizeof(prime_table)/sizeof(prime_table[0]);

//------------------------------------------------------------------------------
int get_digit_usage32(const uint8_t *digit)
{
    if( digit[3] ) return 4;
    if( digit[2] ) return 3;
    if( digit[1] ) return 2;
    if( digit[0] ) return 1;
    return 1;
}
//------------------------------------------------------------------------------
bool is_uintbig_equal_to_int32(const uintbig_t *obj, uint32_t val)
{
    return obj->digit[0] == ( ( val & 0x000000FF ) >>  0) &&
           obj->digit[1] == ( ( val & 0x0000FF00 ) >>  8) &&
           obj->digit[2] == ( ( val & 0x00FF0000 ) >> 16) &&
           obj->digit[3] == ( ( val & 0xFF000000 ) >> 24) &&
           obj->ndig     == get_digit_usage32(obj->digit);
}
//------------------------------------------------------------------------------
void test_import_and_export(void)
{
    // Import from binary data
    {
        uintbig_t obj;

        obj = uintbig_from_bin("\x00", sizeof("\x00"));
        assert( is_uintbig_equal_to_int32(&obj, 0x00) );

        obj = uintbig_from_bin("\x75", sizeof("\x75"));
        assert( is_uintbig_equal_to_int32(&obj, 0x75) );

        obj = uintbig_from_bin("\x75\xAF", sizeof("\x75\xAF"));
        assert( is_uintbig_equal_to_int32(&obj, 0xAF75) );

        obj = uintbig_from_bin("\x75\xAF\x94", sizeof("\x75\xAF\x94"));
        assert( is_uintbig_equal_to_int32(&obj, 0x94AF75) );

        obj = uintbig_from_bin("\x75\xAF\x94\x68", sizeof("\x75\xAF\x94\x68"));
        assert( is_uintbig_equal_to_int32(&obj, 0x6894AF75) );
    }

    // Import from integer
    {
        uintbig_t obj;
        uint32_t  val;

        val = 0x00;
        obj = uintbig_from_int(val);
        assert( is_uintbig_equal_to_int32(&obj, val) );

        val = 0x75;
        obj = uintbig_from_int(val);
        assert( is_uintbig_equal_to_int32(&obj, val) );

        val = 0xAF75;
        obj = uintbig_from_int(val);
        assert( is_uintbig_equal_to_int32(&obj, val) );

        val = 0x94AF75;
        obj = uintbig_from_int(val);
        assert( is_uintbig_equal_to_int32(&obj, val) );

        val = 0x6894AF75;
        obj = uintbig_from_int(val);
        assert( is_uintbig_equal_to_int32(&obj, val) );
    }

    // Import from string
    {
        uintbig_t obj, obj2;

        // - Invalid input

        assert( uintbig_from_hex(  NULL).ndig == -1 );
        assert( uintbig_from_hex(""    ).ndig == -1 );
        assert( uintbig_from_hex("$#^^").ndig == -1 );

        // - Normal input

        obj  = uintbig_from_hex("00");
        obj2 = uintbig_from_int(0);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("75");
        obj2 = uintbig_from_int(0x75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("AF75");
        obj2 = uintbig_from_int(0xAF75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("94AF75");
        obj2 = uintbig_from_int(0x94AF75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("6894AF75");
        obj2 = uintbig_from_int(0x6894AF75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        // - Special input

        obj  = uintbig_from_hex("0");
        obj2 = uintbig_from_int(0);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("7");
        obj2 = uintbig_from_int(7);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("00000");
        obj2 = uintbig_from_int(0);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("000075");
        obj2 = uintbig_from_int(0x75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("===AF75=7805===");
        obj2 = uintbig_from_int(0xAF75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("=F75=");
        obj2 = uintbig_from_int(0x0F75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );

        obj  = uintbig_from_hex("=44316894AF75=");
        obj2 = uintbig_from_int(0x6894AF75);
        assert( obj.ndig == obj2.ndig && !memcmp(obj.digit, obj2.digit, BIGNUM_DIGSZ) );
    }

    // Export to integer
    {
        uintbig_t obj;

        obj = uintbig_from_int(0x4A7B);
        assert( uintbig_to_int(&obj) == 0x4A7B );
    }

    // Export to string
    {
        uintbig_t obj;
        char      buf[1024];

        obj = uintbig_from_int(0x00940075);

        assert( 0 == uintbig_to_str(NULL,         100, &obj) );
        assert( 0 == uintbig_to_str(buf ,           4, &obj) );
        assert( 0 == uintbig_to_str(buf , sizeof(buf), NULL) );

        assert( 7 == uintbig_to_str(buf, sizeof(buf), &obj) );
        assert( 0 == strncmp(buf, "940075", sizeof(buf)) );
    }
}
//------------------------------------------------------------------------------
bool test_base_shift_left(uint32_t val, size_t n)
{
    uintbig_t obj = uintbig_from_int(val);

    uintbig_shift_left(&obj, n);

    return is_uintbig_equal_to_int32(&obj, val<<n);
}
//------------------------------------------------------------------------------
bool test_base_shift_right(uint32_t val, size_t n)
{
    uintbig_t obj = uintbig_from_int(val);

    uintbig_shift_right(&obj, n);

    return is_uintbig_equal_to_int32(&obj, val>>n);
}
//------------------------------------------------------------------------------
void test_base_operation(void)
{
    int i;

    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);

        assert( !( uintbig_is_equal     (&obja, &obja) ^ ( aa[i] == aa[i] ) ) );
        assert( !( uintbig_is_equal     (&obja, &objb) ^ ( aa[i] == bb[i] ) ) );
        assert( !( uintbig_is_greatthen (&obja, &obja) ^ ( aa[i] >  aa[i] ) ) );
        assert( !( uintbig_is_greatthen (&obja, &objb) ^ ( aa[i] >  bb[i] ) ) );
        assert( !( uintbig_is_lessthen  (&obja, &obja) ^ ( aa[i] <  aa[i] ) ) );
        assert( !( uintbig_is_lessthen  (&obja, &objb) ^ ( aa[i] <  bb[i] ) ) );
        assert( !( uintbig_is_greatequal(&obja, &obja) ^ ( aa[i] >= aa[i] ) ) );
        assert( !( uintbig_is_greatequal(&obja, &objb) ^ ( aa[i] >= bb[i] ) ) );
        assert( !( uintbig_is_lessequal (&obja, &obja) ^ ( aa[i] <= aa[i] ) ) );
        assert( !( uintbig_is_lessequal (&obja, &objb) ^ ( aa[i] <= bb[i] ) ) );
    }

    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_and(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] & bb[i]) );

        objr = uintbig_or(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] | bb[i]) );

        objr = uintbig_xor(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] ^ bb[i]) );
    }

    {
        assert( test_base_shift_left(0xF86497BD,  0) );
        assert( test_base_shift_left(0xF86497BD,  1) );
        assert( test_base_shift_left(0xF86497BD,  5) );
        assert( test_base_shift_left(0xF86497BD,  8) );
        assert( test_base_shift_left(0xF86497BD, 10) );
        assert( test_base_shift_left(0xF86497BD, 16) );
        assert( test_base_shift_left(0xF86497BD, 20) );
        assert( test_base_shift_left(0xF86497BD, 24) );
        assert( test_base_shift_left(0x000497BD, 12) );
        assert( test_base_shift_left(0x00000000,  8) );

        assert( test_base_shift_right(0xF86497BD,  0) );
        assert( test_base_shift_right(0xF86497BD,  1) );
        assert( test_base_shift_right(0xF86497BD,  5) );
        assert( test_base_shift_right(0xF86497BD,  8) );
        assert( test_base_shift_right(0xF86497BD, 10) );
        assert( test_base_shift_right(0xF86497BD, 16) );
        assert( test_base_shift_right(0xF86497BD, 20) );
        assert( test_base_shift_right(0xF86497BD, 24) );
        assert( test_base_shift_right(0xF8649000, 12) );
        assert( test_base_shift_right(0x00000000,  8) );
    }
}
//------------------------------------------------------------------------------
void test_math_basic(void)
{
    int i;

    // Add
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_add(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] + bb[i]) );
    }

    // Subtract
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_sub(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] - bb[i]) );
    }

    // Multiply
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_mul(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] * bb[i]) );
    }

    // Divide and modulo
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objq, objm;

        if( bb[i] == 0 ) continue;

        uintbig_divmod(&obja, &objb, &objq, &objm);
        assert( is_uintbig_equal_to_int32(&objq, aa[i] / bb[i]) );
        assert( is_uintbig_equal_to_int32(&objm, aa[i] % bb[i]) );

        objq = uintbig_div(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objq, aa[i] / bb[i]) );

        objm = uintbig_mod(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objm, aa[i] % bb[i]) );
    }

    // Plus plus
    for(i=0; i<testcnt; ++i)
    {
        uint32_t  val  = aa[i];
        uintbig_t obja = uintbig_from_int(val);
        int       j;

        for(j=0; j<500; ++j)
        {
            uintbig_plusplus(&obja);
            assert( is_uintbig_equal_to_int32(&obja, ++val) );
        }
    }

    // Minus minus
    for(i=0; i<testcnt; ++i)
    {
        uint32_t  val  = aa[i];
        uintbig_t obja = uintbig_from_int(val);
        int       j;

        for(j=0; j<500; ++j)
        {
            uintbig_minusminus(&obja);
            assert( is_uintbig_equal_to_int32(&obja, --val) );
        }
    }
}
//------------------------------------------------------------------------------
void test_math_advanced(void)
{
    int i;

    // Power fo 2
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objr;

        objr = uintbig_power2(&obja);
        assert( is_uintbig_equal_to_int32(&objr, aa[i] * aa[i]) );
    }

    // Power
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_power(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, ans_of_power[i]) );
    }

    // Power and modulo
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objc = uintbig_from_int(777);
        uintbig_t objr;

        if( bb[i] != 0 )
        {
            objr = uintbig_powermod(&obja, &objb, &objc);
            assert( is_uintbig_equal_to_int32(&objr, ans_of_powermod[i]) );
        }
    }

    // Square root
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objr;
        uintbig_t objr2;   // r*r
        uintbig_t objr3;   // ( r + 1 )*( r + 1 )

        objr  = uintbig_sqrt(&obja);

        objr2 = uintbig_power2(&objr);
        objr3 = uintbig_add(&objr, &uintbig_const_one);
        objr3 = uintbig_power2(&objr3);

        assert( uintbig_is_lessequal(&objr2, &obja) );
        assert( uintbig_is_greatthen(&objr3, &obja) ||
                uintbig_is_lessthen (&objr3, &objr2) );
    }

    // Greatest common divisor
    for(i=0; i<testcnt; ++i)
    {
        uintbig_t obja = uintbig_from_int(aa[i]);
        uintbig_t objb = uintbig_from_int(bb[i]);
        uintbig_t objr;

        objr = uintbig_gcd(&obja, &objb);
        assert( is_uintbig_equal_to_int32(&objr, gcd_of_aa_and_bb[i]) );
    }

    // Modular multiplicative inverse
    for(i=0; i<65536; ++i)
    {
        uintbig_t objn = uintbig_from_int(277);
        uintbig_t obja = uintbig_from_int(i);
        uintbig_t objr;

        if( uintbig_modmulinv(&obja, &objn, &objr) )
        {
            uintbig_t temp;

            temp = uintbig_mul(&obja, &objr);
            temp = uintbig_mod(&temp, &objn);
            assert( is_uintbig_equal_to_int32(&temp, 1) );
        }
    }
}
//------------------------------------------------------------------------------
bool test_rand_digs(const uintbig_t *obj, size_t ndig)
{
    assert( obj );

    switch( ndig )
    {
    case 0:  return  ( obj->ndig == 1 ) &&
                    !( obj->digit[0]  ) &&
                    !( obj->digit[1]  ) &&
                    !( obj->digit[2]  ) &&
                    !( obj->digit[3]  );
    case 1:  return  ( obj->ndig == 1 ) &&
                     ( obj->digit[0]  ) &&
                    !( obj->digit[1]  ) &&
                    !( obj->digit[2]  ) &&
                    !( obj->digit[3]  );
    case 2:  return  ( obj->ndig == 2 ) &&
                     ( obj->digit[0]  ) &&
                     ( obj->digit[1]  ) &&
                    !( obj->digit[2]  ) &&
                    !( obj->digit[3]  );
    case 3:  return  ( obj->ndig == 3 ) &&
                     ( obj->digit[0]  ) &&
                     ( obj->digit[1]  ) &&
                     ( obj->digit[2]  ) &&
                    !( obj->digit[3]  );
    case 4:  return  ( obj->ndig == 4 ) &&
                     ( obj->digit[0]  ) &&
                     ( obj->digit[1]  ) &&
                     ( obj->digit[2]  ) &&
                     ( obj->digit[3]  );
    default: return false;
    }
}
//------------------------------------------------------------------------------
void test_math_random_and_prime(void)
{
    int i, n;

    // Random number

    for(n=0; n<=4; ++n)
    {
        int succ_cnt = 0;

        for(i=0; i<50; ++i)
        {
            uintbig_t a = uintbig_rand_ndig(n, NULL);
            if( test_rand_digs(&a, n) )
                ++succ_cnt;
        }
        assert( succ_cnt >= 40 );
    }

    for(i=0; i<50; ++i)
    {
        uintbig_t minval = uintbig_from_int(0x0DA6);
        uintbig_t maxval = uintbig_from_int(0x94BA25);
        uintbig_t rndval;

        rndval = uintbig_rand_range(&minval, &maxval, NULL);
        assert( uintbig_is_greatequal(&rndval, &minval) &&
                uintbig_is_lessthen  (&rndval, &maxval) );
    }

    // Prime number

    for(i=100; i<1000; ++i)
    {
        uintbig_t a = uintbig_from_int(i);
        int       j;
        bool      prime;

        prime = false;
        for(j=0; j<prime_table_cnt; ++j)
        {
            if( i == prime_table[j] )
            {
                prime = true;
                break;
            }
        }

        if( prime ) assert(  uintbig_prime_test(&a, 32, NULL) );
        else        assert( !uintbig_prime_test(&a, 32, NULL) );
    }
}
//------------------------------------------------------------------------------
int main(void)
{
    assert( sizeof(aa) == sizeof(bb) );
    assert( sizeof(aa) == sizeof(ans_of_power) );
    assert( sizeof(aa) == sizeof(ans_of_powermod) );
    assert( sizeof(aa) == sizeof(gcd_of_aa_and_bb) );

    test_import_and_export();
    test_base_operation();
    test_math_basic();
    test_math_advanced();
    test_math_random_and_prime();

    return 0;
}
//------------------------------------------------------------------------------

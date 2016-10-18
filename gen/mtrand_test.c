/*
 * Mersenne Twister Random Number Generator (mtrand) 測試程式
 */
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "mtrand.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

// Test resources for 32 bits Mersenne Twister random number generator

const uint32_t test_seed_32 = 1;

const uint32_t test_res_32[] =
{
    0xC54C3404,
    0xFAD32138,
    0x09BA58F2,
    0xA15E4046,
    0x49E2F06D,
    0x27D23B57,
    0x4C2BADEC,
    0x7D516B7F,
    0xC664E6BF,
    0xDDBE271D,
    0x99467C9E,
    0x918BE989,
    0x7A54F0F6,
    0xBD77DBF7,
    0x3F81D1D9,
    0x1CBAA141,
    0xA3463F00,
    0x24EE988E,
    0x321D604F,
    0x6F2E10EB,
    0xD2674AC9,
    0x1E0D916E,
    0x8F833A2F,
    0x4F120421,
    0x5ECDFD15,
    0xF85CD8D8,
    0x68A2A708,
    0xB43AF12C,
    0x914FB092,
    0x1404FC7F,
    0xD23E6BB1,
    0x5224C4FE,
    0xD9D3CB92,
    0x48EDEB19,
    0xF2451A01,
    0x15915A5C,
    0x78705B1A,
    0xC01C41A5,
    0xC4E6D144,
    0xCDBC01E5,
    0xC9CF02BE,
    0x698D09EF,
    0xCE9E0EE1,
    0x00AB900E,
    0x18E92276,
    0xC6BC2644,
    0xC5027777,
    0x8C16D882,
    0x4C30A0CE,
    0x4B6AC238
};

const int test_cnt_32 = sizeof(test_res_32)/sizeof(test_res_32[0]);

// Test resources for 64 bits Mersenne Twister random number generator

const uint64_t test_seed_64 = 1;

const uint64_t test_res_64[] =
{
    0x2245BD5FBB686F68,
    0x22EB92502318FA4E,
    0x7382D1E77AE6459A,
    0x0561D8057935C08E,
    0x59D47572ECFC6738,
    0xE94EC2D2B9936849,
    0x78833635915BD1B4,
    0x130D84F91BF14B09,
    0x91E180B364F46100,
    0xA29E835C0E448010,
    0x16E6678D39FEEF00,
    0x8E61BD8674B6331B,
    0xCA26A1A840991E65,
    0x38BCFC057E745A63,
    0x6B2DDC59760D03DC,
    0x3FF1722C566A5199,
    0x4AB7A473D29376C1,
    0xCDA0E546AAF708BA,
    0x797EFACA4DBD97E3,
    0x451AC15E436FCBE8,
    0x493A09523AFA6FCF,
    0xBFBDDC1F91C5BF67,
    0x7547A68CFDD04144,
    0x4E6240030EECC15B,
    0x525ECDF403233263,
    0x1CF8FA026C9F0D1A,
    0x1E8DEE4C09D96C4B,
    0x11B1C7962DB13031,
    0xB1DBD9F06D472B18,
    0xA5D601951A58A5CA,
    0xCA4AE8E06A9888A7,
    0x647C72C63386BCB5,
    0x87A9F8B641C7EB81,
    0x65FB9C2170DAA714,
    0x30BB3E55B6D6C3EE,
    0x98D462C4EA0C48BC,
    0xE36F837B19707512,
    0x5EDFFFB0A1180E13,
    0x09D7152814D89685,
    0x48840C169F64A1FC,
    0xE2A351C63E427C90,
    0x4303AC94BA574D30,
    0x853E8231C1113570,
    0x04FF591B35F3337D,
    0x81B650B9A9C4FCFB,
    0xFFB93DBFBB453BEA,
    0x9BD408A4CD4CCB6F,
    0xEE2AA42AE3E48E6F,
    0xDF508714E111F47E,
    0x297D4ED47B672B52
};

const int test_cnt_64 = sizeof(test_res_64)/sizeof(test_res_64[0]);

// Main test process

bool check_rand_bin(const uint8_t *data1, const uint8_t *data2, size_t bufsz, size_t rndsz)
{
    int i;

    assert( data1 && data2 && bufsz && rndsz <= bufsz );

    for(i=0; i<rndsz; ++i)
    {
        if( !data1[i] && !data2[i] ) return false;
    }
    for(i=rndsz; i<bufsz; ++i)
    {
        if( data1[i] || data2[i] ) return false;
    }

    return true;
}

int main(void)
{
    int i;

    mt_srand32(NULL, test_seed_32);
    for(i=0; i<test_cnt_32; ++i)
    {
        uint32_t val = mt_rand32(NULL);
        assert( val == test_res_32[i] );
    }

    mt_srand64(NULL, test_seed_64);
    for(i=0; i<50; ++i)
    {
        uint64_t val = mt_rand64(NULL);
        assert( val == test_res_64[i] );
    }

    mt_srandbin(NULL, test_seed_64);
    for(i=1; i<=30; ++i)
    {
        uint8_t buf1[30];
        uint8_t buf2[30];

        memset(buf1, 0, sizeof(buf1));
        memset(buf2, 0, sizeof(buf2));
        mt_randbin(NULL, buf1, i);
        mt_randbin(NULL, buf2, i);
        assert( check_rand_bin(buf1, buf2, sizeof(buf1), i) );
    }

    return 0;
}

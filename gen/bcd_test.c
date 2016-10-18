#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "bcd.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

int main(int argc, char *argv[])
{
    uint8_t bcddata[64];

    // Encode test

    memset(bcddata, -1, sizeof(bcddata));
    bcd_encode(bcddata, 8, 13572468);
    assert( 0 == memcmp(bcddata, "\x00\x00\x00\x00\x13\x57\x24\x68", 8) );

    memset(bcddata, -1, sizeof(bcddata));
    bcd_encode(bcddata, 4, 13572468);
    assert( 0 == memcmp(bcddata, "\x13\x57\x24\x68", 4) );

    memset(bcddata, -1, sizeof(bcddata));
    bcd_encode(bcddata, 2, 13572468);
    assert( 0 == memcmp(bcddata, "\x24\x68", 2) );

    // Decode test

    assert( 13572468 == bcd_decode("\x00\x00\x00\x00\x13\x57\x24\x68", 8) );

    return 0;
}

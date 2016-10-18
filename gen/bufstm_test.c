#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "bufstm.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

void bufistm_test(void)
{
    static const uint8_t buffer[] = { 0x00,0x11,0x22, 0x33,0x44,0x55, 0x66,0x77,0x88, 0x99,0xAA, };
    bufistm_t stream;
    uint8_t   data[64];

    // Normal read

    bufistm_init(&stream, buffer, sizeof(buffer));
    assert( bufistm_get_restsize(&stream) == sizeof(buffer) );

    assert( !bufistm_read(&stream, NULL, 3) );                              // Invalid input
    assert(  bufistm_get_restsize(&stream) == sizeof(buffer) );
    assert(  bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );

    assert( bufistm_read(&stream, data, 0) );                               // Read zero size
    assert( bufistm_get_restsize(&stream) == sizeof(buffer) );
    assert( bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );

    assert( bufistm_read(&stream, data, 3) );
    assert( bufistm_get_restsize(&stream) == sizeof(buffer) - 3 );
    assert( bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );
    assert( 0 == memcmp(data, "\x00\x11\x22", 3) );

    memcpy(data, bufistm_get_buf(&stream), 3);                              // Read data from buffer directly
    assert( bufistm_commit_read(&stream, 3) );                              //
    assert( bufistm_get_restsize(&stream) == sizeof(buffer) - 3 - 3 );
    assert( bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );
    assert( 0 == memcmp(data, "\x33\x44\x55", 3) );

    assert( bufistm_read(&stream, data, 3) );
    assert( bufistm_get_restsize(&stream) == sizeof(buffer) - 3 - 3 - 3 );
    assert( bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );
    assert( 0 == memcmp(data, "\x66\x77\x88", 3) );

    assert( !bufistm_read(&stream, data, 3) );                              // No more data
    assert(  bufistm_get_restsize(&stream) == sizeof(buffer) - 3 - 3 - 3 );
    assert(  bufistm_get_restsize(&stream) + bufistm_get_readsize(&stream) == sizeof(buffer) );

    // Invalid buffer

    bufistm_init(&stream, NULL, sizeof(buffer));
    assert( bufistm_get_restsize(&stream) == 0 );
    assert( bufistm_get_readsize(&stream) == 0 );

    assert( !bufistm_read(&stream, data, 3) );
    assert(  bufistm_get_restsize(&stream) == 0 );
    assert(  bufistm_get_readsize(&stream) == 0 );
}

void bufostm_test(void)
{
    uint8_t   buffer[11];
    bufostm_t stream;

    memset(buffer, 0, sizeof(buffer));

    // Normal write

    bufostm_init(&stream, buffer, sizeof(buffer));
    assert( bufostm_get_datasize(&stream) == 0 );

    assert( bufostm_write(&stream, "\x00\x11\x22", 3) );
    assert( bufostm_get_datasize(&stream) == 3 );

    memcpy(bufostm_get_buf(&stream), "\x33\x44\x55", 3);        // Write data to buffer directly
    assert( bufostm_commit_write(&stream, 3) );                 //
    assert( bufostm_get_datasize(&stream) == 3 + 3 );

    assert( bufostm_write(&stream, "\x66\x77\x88", 3) );
    assert( bufostm_get_datasize(&stream) == 3 + 3 + 3 );

    assert( !bufostm_write(&stream, "\x99\xAA\xBB", 3) );       // Full up
    assert(  bufostm_get_datasize(&stream) == 3 + 3 + 3 );

    assert( !bufostm_write(&stream, NULL, 1) );                 // Invalid input
    assert(  bufostm_get_datasize(&stream) == 3 + 3 + 3 );

    assert( bufostm_write(&stream, "", 0) );                    // Write zero size
    assert( bufostm_get_datasize(&stream) == 3 + 3 + 3 );

    assert( bufostm_putbyte(&stream, 0x99) );
    assert( bufostm_get_datasize(&stream) == 3 + 3 + 3 + 1 );

    assert( bufostm_putbyte(&stream, 0xAA) );
    assert( bufostm_get_datasize(&stream) == 3 + 3 + 3 + 2 );

    assert( !bufostm_putbyte(&stream, 0xBB) );                  // Full up
    assert(  bufostm_get_datasize(&stream) == 3 + 3 + 3 + 2 );

    assert( 0 == memcmp(buffer, "\x00\x11\x22""\x33\x44\x55""\x66\x77\x88""\x99\xAA", 3+3+3+2) );

    // Invalid buffer

    bufostm_init(&stream, NULL, sizeof(buffer));
    assert( bufostm_get_datasize(&stream) == 0 );

    assert( !bufostm_write(&stream, "\x00\x11\x22", 3) );
    assert(  bufostm_get_datasize(&stream) == 0 );

    assert( !bufostm_putbyte(&stream, 0xAA) );
    assert(  bufostm_get_datasize(&stream) == 0 );
}

int main(void)
{
    bufistm_test();
    bufostm_test();

    return 0;
}

#include "bcd.h"

//------------------------------------------------------------------------------
static
uint8_t digits_to_byte(uint64_t digits)
{
    return ( ( digits / 10 % 10 ) << 4 ) | ( digits % 10 );
}
//------------------------------------------------------------------------------
static
unsigned byte_to_digit(uint8_t byteval)
{
    return 10*( byteval >> 4 ) + ( byteval & 0x0F );
}
//------------------------------------------------------------------------------
void bcd_encode(void *buf, size_t bufsize, uint64_t value)
{
    /**
     * @brief Encode an integer value to BCD data.
     *
     * @param buf     A buffer to receive the encoded data.
     * @param bufsize Size of the output data.
     * @param value   The input integer value.
     */
    uint8_t *bufpos = buf;

    if( !bufpos ) return;

    for( bufpos += bufsize - 1;
         bufsize;
         --bufsize, --bufpos )
    {
        *bufpos  = digits_to_byte(value);
        value   /= 100;
    }
}
//------------------------------------------------------------------------------
uint64_t bcd_decode(const void /*in*/ *data, size_t /*in*/ size)
{
    /**
     * @brief Decode BCD data to an integer.
     *
     * @param data BCD data input.
     * @param size Size of the BCD data.
     * @return The decoded integer value.
     */
    const uint8_t *datapos = data;
    uint64_t       res     = 0;

    if( !datapos ) return 0;

    while( size-- )
    {
        res *= 100;
        res += byte_to_digit(*datapos++);
    }

    return res;
}
//------------------------------------------------------------------------------

#include <assert.h>
#include <string.h>
#include "../endian.h"
#include "../minmax.h"
#include "bitrot.h"
#include "sha.h"

typedef union chunk_t
{
    uint8_t  bytes[64];
    uint32_t words[16];
} chunk_t;

typedef struct md5stat_t
{
    chunk_t  chunk;
    uint32_t state[4];
    uint64_t accsize;
    bool     use_short_version;
} md5stat_t;

static const int s[64] =
{
    7, 12, 17, 22,   7, 12, 17, 22,   7, 12, 17, 22,   7, 12, 17, 22,
    5,  9, 14, 20,   5,  9, 14, 20,   5,  9, 14, 20,   5,  9, 14, 20,
    4, 11, 16, 23,   4, 11, 16, 23,   4, 11, 16, 23,   4, 11, 16, 23,
    6, 10, 15, 21,   6, 10, 15, 21,   6, 10, 15, 21,   6, 10, 15, 21
};

static const uint32_t k[64] =
{
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

static const uint32_t state0[4] =
{
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
};

//------------------------------------------------------------------------------
static
void peek_loop_coeff(int       i,
                     uint32_t  a,
                     uint32_t  b,
                     uint32_t  c,
                     uint32_t  d,
                     uint32_t *f,
                     uint32_t *g)
{
    assert( 0 <= i && i < 64 );

    if( i < 16 )
    {
        *f = ( b & c ) | ( ~b & d );
        *g = i;
    }
    else if( i < 32 )
    {
        *f = ( d & b ) | ( ~d & c );
        *g = ( 5*i + 1 ) & 0x0F;
    }
    else if( i < 48 )
    {
        *f = b ^ c ^ d;
        *g = ( 3*i + 5 ) & 0x0F;
    }
    else if( i <= 64 )
    {
        *f = c ^ ( b | ~d );
        *g = ( 7*i ) & 0x0F;
    }
}
//------------------------------------------------------------------------------
static
void md5_transform(uint32_t state[8], const chunk_t *chunk)
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];

    for(int i=0; i<64; ++i)
    {
        uint32_t f, g;
        peek_loop_coeff(i, a, b, c, d, &f, &g);

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + bitrot32_left( a + f + k[i] + chunk->words[g], s[i] );
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}
//------------------------------------------------------------------------------
static
unsigned modulo64(size_t value)
{
    return value & 0x3F;
}
//------------------------------------------------------------------------------
static
unsigned modulo_chunk_size(size_t value)
{
    return modulo64(value);
}
//------------------------------------------------------------------------------
static
void md5stat_init(md5stat_t *stat)
{
    memset(stat, 0, sizeof(*stat));
    memcpy(stat->state, state0, sizeof(state0));
}
//------------------------------------------------------------------------------
static
size_t md5stat_update(md5stat_t *stat, const void *data, size_t size)
{
    unsigned offset   = modulo_chunk_size(stat->accsize);
    unsigned bufsize  = sizeof(chunk_t) - offset;
    unsigned fillsize = MIN( bufsize, size );

    memcpy( stat->chunk.bytes + offset, data, fillsize );
    stat->accsize += fillsize;

    // Check if chunk full.
    if( 0 == modulo_chunk_size(stat->accsize) )
        md5_transform(stat->state, &stat->chunk);

    return fillsize;
}
//------------------------------------------------------------------------------
static
void md5stat_append_bit1(md5stat_t *stat)
{
    md5stat_update(stat, "\x80", 1);
}
//------------------------------------------------------------------------------
static
void md5stat_append_zeros(md5stat_t *stat)
{
    static const uint8_t zeros[sizeof(chunk_t)] = {0};
    unsigned count = sizeof(chunk_t) - modulo_chunk_size( stat->accsize + sizeof(uint64_t) );

    while(count)
    {
        unsigned fillsize = md5stat_update(stat, zeros, count);
        assert( fillsize <= count );

        count -= fillsize;
    }
}
//------------------------------------------------------------------------------
static
void md5stat_append_accsize(md5stat_t *stat, uint64_t msgsize)
{
    union
    {
        uint8_t  bytes[8];
        uint64_t value;
    } exg;

    uint64_t bitsize = msgsize << 3;

    exg.value = endian_local_to_le_64(bitsize);
    md5stat_update(stat, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void md5stat_get_hash_result(const md5stat_t *stat, void *hashval)
{
    union
    {
        uint8_t  bytes[16];
        uint32_t state[4];
    } exg;

    for(int i=0; i<4; ++i)
        exg.state[i] = endian_local_to_le_32(stat->state[i]);

    memcpy(hashval, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void md5stat_final(md5stat_t *stat, void *hashval)
{
    uint64_t msgsize = stat->accsize;

    md5stat_append_bit1(stat);
    md5stat_append_zeros(stat);
    md5stat_append_accsize(stat, msgsize);
    assert( 0 == modulo_chunk_size(stat->accsize) );

    md5stat_get_hash_result(stat, hashval);
}
//------------------------------------------------------------------------------
void md5(void *dest, const void *src, size_t size)
{
    /**
     * MD5 hashing function.
     *
     * @param dest A buffer to receive the 16-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    assert( dest && src );

    md5stat_t stat;
    md5stat_init(&stat);

    const uint8_t *pos = src;
    while(size)
    {
        unsigned fillsize = md5stat_update(&stat, pos, size);
        assert( fillsize <= size );

        pos  += fillsize;
        size -= fillsize;
    }

    md5stat_final(&stat, dest);
}
//------------------------------------------------------------------------------

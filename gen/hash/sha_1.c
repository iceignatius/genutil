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

typedef struct shastat_t
{
    chunk_t  chunk;
    uint32_t state[5];
    uint64_t accsize;
} shastat_t;

static const uint32_t state0[5] =
{
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0,
};

//------------------------------------------------------------------------------
static
void peek_loop_coeff(int       i,
                     uint32_t  a,
                     uint32_t  b,
                     uint32_t  c,
                     uint32_t  d,
                     uint32_t  e,
                     uint32_t *f,
                     uint32_t *k)
{
    assert( 0 <= i && i < 80 );

    if( i < 20 )
    {
        *f = ( b & c ) | ( ~b & d );
        *k = 0x5A827999;
    }
    else if( i < 40 )
    {
        *f = b ^ c ^ d;
        *k = 0x6ED9EBA1;
    }
    else if( i < 60 )
    {
        *f = ( b & c ) | ( b & d ) | ( c & d );
        *k = 0x8F1BBCDC;
    }
    else if( i < 80 )
    {
        *f = b ^ c ^ d;
        *k = 0xCA62C1D6;
    }
}
//------------------------------------------------------------------------------
static
void sha_transform(uint32_t state[5], const chunk_t *chunk)
{
    uint32_t w[80];
    for(int i=0; i<16; ++i)
        w[i] = endian_be_to_local_32(chunk->words[i]);
    for(int i=16; i<80; ++i)
        w[i] = bitrot32_left( w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1 );

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    for(int i=0; i<80; ++i)
    {
        uint32_t f, k;
        peek_loop_coeff(i, a, b, c, d, e, &f, &k);

        uint32_t temp = bitrot32_left(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = bitrot32_left(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
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
void shastat_init(shastat_t *stat)
{
    memset(stat, 0, sizeof(*stat));
    memcpy(stat->state, state0, sizeof(state0));
}
//------------------------------------------------------------------------------
static
size_t shastat_update(shastat_t *stat, const void *data, size_t size)
{
    unsigned offset   = modulo_chunk_size(stat->accsize);
    unsigned bufsize  = sizeof(chunk_t) - offset;
    unsigned fillsize = MIN( bufsize, size );

    memcpy( stat->chunk.bytes + offset, data, fillsize );
    stat->accsize += fillsize;

    // Check if chunk full.
    if( 0 == modulo_chunk_size(stat->accsize) )
        sha_transform(stat->state, &stat->chunk);

    return fillsize;
}
//------------------------------------------------------------------------------
static
void shastat_append_bit1(shastat_t *stat)
{
    shastat_update(stat, "\x80", 1);
}
//------------------------------------------------------------------------------
static
void shastat_append_zeros(shastat_t *stat)
{
    static const uint8_t zeros[sizeof(chunk_t)] = {0};
    unsigned count = sizeof(chunk_t) - modulo_chunk_size( stat->accsize + sizeof(uint64_t) );

    while(count)
    {
        unsigned fillsize = shastat_update(stat, zeros, count);
        assert( fillsize <= count );

        count -= fillsize;
    }
}
//------------------------------------------------------------------------------
static
void shastat_append_accsize(shastat_t *stat, uint64_t msgsize)
{
    union
    {
        uint8_t  bytes[8];
        uint64_t value;
    } exg;

    uint64_t bitsize = msgsize << 3;

    exg.value = endian_local_to_be_64(bitsize);
    shastat_update(stat, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void shastat_get_hash_result(const shastat_t *stat, void *hashval)
{
    union
    {
        uint8_t  bytes[20];
        uint32_t state[5];
    } exg;

    for(int i=0; i<5; ++i)
        exg.state[i] = endian_local_to_be_32(stat->state[i]);
    memcpy(hashval, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void shastat_final(shastat_t *stat, void *hashval)
{
    uint64_t msgsize = stat->accsize;

    shastat_append_bit1(stat);
    shastat_append_zeros(stat);
    shastat_append_accsize(stat, msgsize);
    assert( 0 == modulo_chunk_size(stat->accsize) );

    shastat_get_hash_result(stat, hashval);
}
//------------------------------------------------------------------------------
void sha_1(void *dest, const void *src, size_t size)
{
    /**
     * SHA-1 160-bits hashing function.
     *
     * @param dest A buffer to receive the 20-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    assert( dest && src );

    shastat_t stat;
    shastat_init(&stat);

    const uint8_t *pos = src;
    while(size)
    {
        unsigned fillsize = shastat_update(&stat, pos, size);
        assert( fillsize <= size );

        pos  += fillsize;
        size -= fillsize;
    }

    shastat_final(&stat, dest);
}
//------------------------------------------------------------------------------

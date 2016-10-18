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
    uint32_t state[8];
    uint64_t accsize;
    bool     use_short_version;
} shastat_t;

static const uint32_t k[64] =
{
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, 0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, 0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, 0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static const uint32_t state0_for_normal[8] =
{
    0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

static const uint32_t state0_for_short[8] =
{
    0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939, 0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4
};

//------------------------------------------------------------------------------
static
void sha_transform(uint32_t state[8], const chunk_t *chunk)
{
    uint32_t w[64];
    for(int i=0; i<16; ++i)
    {
        w[i] = endian_be_to_local_32(chunk->words[i]);
    }
    for(int i=16; i<64; ++i)
    {
        uint32_t s0 = bitrot32_right(w[i-15],  7) ^ bitrot32_right(w[i-15], 18) ^ ( w[i-15] >> 3  );
        uint32_t s1 = bitrot32_right(w[i- 2], 17) ^ bitrot32_right(w[i- 2], 19) ^ ( w[i- 2] >> 10 );
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];
    uint32_t f = state[5];
    uint32_t g = state[6];
    uint32_t h = state[7];

    for(int i=0; i<64; ++i)
    {
        uint32_t s1    = bitrot32_right(e, 6) ^ bitrot32_right(e, 11) ^ bitrot32_right(e, 25);
        uint32_t ch    = ( e & f ) ^ ( ~e & g );
        uint32_t temp1 = h + s1 + ch + k[i] + w[i];
        uint32_t s0    = bitrot32_right(a, 2) ^ bitrot32_right(a, 13) ^ bitrot32_right(a, 22);
        uint32_t maj   = ( a & b ) ^ ( a & c ) ^ ( b & c );
        uint32_t temp2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
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
void shastat_init(shastat_t *stat, bool use_short_version)
{
    memset(stat, 0, sizeof(*stat));

    stat->use_short_version = use_short_version;
    if( use_short_version )
        memcpy(stat->state, state0_for_short, sizeof(state0_for_short));
    else
        memcpy(stat->state, state0_for_normal, sizeof(state0_for_normal));
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
void shastat_get_hash_result(const shastat_t *stat, byte_t *hashval)
{
    union
    {
        uint8_t  bytes[32];
        uint32_t state[8];
    } exg;

    for(int i=0; i<8; ++i)
        exg.state[i] = endian_local_to_be_32(stat->state[i]);

    if( stat->use_short_version )
        memcpy(hashval, exg.bytes, sizeof(exg.bytes)-sizeof(uint32_t));
    else
        memcpy(hashval, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void shastat_final(shastat_t *stat, byte_t *hashval)
{
    uint64_t msgsize = stat->accsize;

    shastat_append_bit1(stat);
    shastat_append_zeros(stat);
    shastat_append_accsize(stat, msgsize);
    assert( 0 == modulo_chunk_size(stat->accsize) );

    shastat_get_hash_result(stat, hashval);
}
//------------------------------------------------------------------------------
static
void sha_256_private(void *dest, const void *src, size_t size, bool use_short_version)
{
    assert( dest && src );

    shastat_t stat;
    shastat_init(&stat, use_short_version);

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
void sha_224(void *dest, const void *src, size_t size)
{
    /**
     * SHA 224-bits hashing function.
     *
     * @param dest A buffer to receive the 28-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    sha_256_private(dest, src, size, true);
}
//------------------------------------------------------------------------------
void sha_256(void *dest, const void *src, size_t size)
{
    /**
     * SHA 256-bits hashing function.
     *
     * @param dest A buffer to receive the 32-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    sha_256_private(dest, src, size, false);
}
//------------------------------------------------------------------------------

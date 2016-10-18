#include <assert.h>
#include <string.h>
#include "../endian.h"
#include "../minmax.h"
#include "bitrot.h"
#include "sha.h"

typedef union chunk_t
{
    uint8_t  bytes[128];
    uint64_t words[16];
} chunk_t;

typedef struct shastat_t
{
    chunk_t  chunk;
    uint64_t state[8];
    uint64_t accsize;
    bool     use_short_version;
} shastat_t;

static const uint64_t k[80] =
{
    0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC,
    0x3956C25BF348B538, 0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118,
    0xD807AA98A3030242, 0x12835B0145706FBE, 0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
    0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235, 0xC19BF174CF692694,
    0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
    0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5,
    0x983E5152EE66DFAB, 0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4,
    0xC6E00BF33DA88FC2, 0xD5A79147930AA725, 0x06CA6351E003826F, 0x142929670A0E6E70,
    0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
    0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B,
    0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30,
    0xD192E819D6EF5218, 0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8,
    0x19A4C116B8D2D0C8, 0x1E376C085141AB53, 0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8,
    0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3,
    0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
    0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B,
    0xCA273ECEEA26619C, 0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178,
    0x06F067AA72176FBA, 0x0A637DC5A2C898A6, 0x113F9804BEF90DAE, 0x1B710B35131C471B,
    0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC, 0x431D67C49C100D4C,
    0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817
};

static const uint64_t state0_for_normal[8] =
{
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1,
    0x510E527FADE682D1, 0x9B05688C2B3E6C1F, 0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179
};

static const uint64_t state0_for_short[8] =
{
    0xCBBB9D5DC1059ED8, 0x629A292A367CD507, 0x9159015A3070DD17, 0x152FECD8F70E5939,
    0x67332667FFC00B31, 0x8EB44A8768581511, 0xDB0C2E0D64F98FA7, 0x47B5481DBEFA4FA4
};

//------------------------------------------------------------------------------
static
void sha_transform(uint64_t state[8], const chunk_t *chunk)
{
    uint64_t w[80];
    for(int i=0; i<16; ++i)
    {
        w[i] = endian_be_to_local_64(chunk->words[i]);
    }
    for(int i=16; i<80; ++i)
    {
        uint64_t s0 = bitrot64_right(w[i-15],  1) ^ bitrot64_right(w[i-15],  8) ^ ( w[i-15] >> 7 );
        uint64_t s1 = bitrot64_right(w[i- 2], 19) ^ bitrot64_right(w[i- 2], 61) ^ ( w[i- 2] >> 6 );
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }

    uint64_t a = state[0];
    uint64_t b = state[1];
    uint64_t c = state[2];
    uint64_t d = state[3];
    uint64_t e = state[4];
    uint64_t f = state[5];
    uint64_t g = state[6];
    uint64_t h = state[7];

    for(int i=0; i<80; ++i)
    {
        uint64_t s1    = bitrot64_right(e, 14) ^ bitrot64_right(e, 18) ^ bitrot64_right(e, 41);
        uint64_t ch    = ( e & f ) ^ ( ~e & g );
        uint64_t temp1 = h + s1 + ch + k[i] + w[i];
        uint64_t s0    = bitrot64_right(a, 28) ^ bitrot64_right(a, 34) ^ bitrot64_right(a, 39);
        uint64_t maj   = ( a & b ) ^ ( a & c ) ^ ( b & c );
        uint64_t temp2 = s0 + maj;

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
unsigned modulo128(size_t value)
{
    return value & 0x7F;
}
//------------------------------------------------------------------------------
static
unsigned modulo_chunk_size(size_t value)
{
    return modulo128(value);
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
    unsigned count = sizeof(chunk_t) - modulo_chunk_size( stat->accsize + 2*sizeof(uint64_t) );

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

    uint64_t bitsize_high = msgsize >> ( 64 - 3 );
    uint64_t bitsize_low  = msgsize << 3;

    exg.value = endian_local_to_be_64(bitsize_high);
    shastat_update(stat, exg.bytes, sizeof(exg.bytes));

    exg.value = endian_local_to_be_64(bitsize_low);
    shastat_update(stat, exg.bytes, sizeof(exg.bytes));
}
//------------------------------------------------------------------------------
static
void shastat_get_hash_result(const shastat_t *stat, byte_t *hashval)
{
    union
    {
        uint8_t  bytes[64];
        uint64_t state[8];
    } exg;

    for(int i=0; i<8; ++i)
        exg.state[i] = endian_local_to_be_64(stat->state[i]);

    if( stat->use_short_version )
        memcpy(hashval, exg.bytes, sizeof(exg.bytes)-sizeof(uint64_t));
    else
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
static
void sha_512_private(void *dest, const void *src, size_t size, bool use_short_version)
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
void sha_384(void *dest, const void *src, size_t size)
{
    /**
     * SHA 384-bits hashing function.
     *
     * @param dest A buffer to receive the 48-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    sha_512_private(dest, src, size, true);
}
//------------------------------------------------------------------------------
void sha_512(void *dest, const void *src, size_t size)
{
    /**
     * SHA 512-bits hashing function.
     *
     * @param dest A buffer to receive the 64-bytes hash data.
     * @param src  Data to calculate.
     * @param size Size of input data.
     */
    sha_512_private(dest, src, size, false);
}
//------------------------------------------------------------------------------

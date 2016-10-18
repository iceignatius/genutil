#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "mtrand.h"

// Constants

#define MM32       397
#define MATRIX_A32 0x9908B0DF
#define MASKU32    0x80000000
#define MASKL32    0x7FFFFFFF
#define DEFSEED32  5489

#define MM64       156
#define MATRIX_A64 0xB5026F5AA96619E9
#define MASKU64    0xFFFFFFFF80000000
#define MASKL64    0x000000007FFFFFFF
#define DEFSEED64  5489

// Shared/Default objects

static mt_rand32_t shrd32 = { {0}, NULL, NULL, NULL, NULL };
static mt_rand64_t shrd64 = { {0}, NULL, NULL, NULL, NULL };

//------------------------------------------------------------------------------
void mt_srand32(mt_rand32_t* /*optional*/ RESTRICT obj, uint32_t seed)
{
    /**
     * @brief Set random seed for 32-bits random number generator.
     * @param obj  Generator object.
     *             This parameter can be NULL if no generator object assigned,
     *             and a common object will be used automatically.
     * @param seed The random seed to initialize the generator.
     */
    int i;

    if( !obj ) obj = &shrd32;  // 若使用者不傳入指定的物件則使用預設的全域物件

    obj->state[0] = seed;
    for(i=1; i<MTRND32_MTXSZ; ++i)
        obj->state[i] = 0x6C078965 * ( obj->state[i-1] ^ ( obj->state[i-1] >> 30 ) ) + i;

    obj->end  = obj->state + MTRND32_MTXSZ;
    obj->curr = obj->state;
    obj->next = obj->state + 1;
    obj->mm   = obj->state + MM32;
}
//------------------------------------------------------------------------------
uint32_t mt_rand32(mt_rand32_t* /*optional*/ RESTRICT obj)
{
    /**
     * @brief Generate 32-bits random number.
     * @param obj Generator object.
     *            This parameter can be NULL if no generator object assigned,
     *            and a common object will be used automatically.
     * @return The random number generated.
     */
    static const uint32_t mat[2] = { 0, MATRIX_A32 };
    uint32_t r;

    if( !obj ) obj = &shrd32;  // 若使用者不傳入指定的物件則使用預設的全域物件

    // Initialize object if it is not initailized

    if( obj->end != obj->state + MTRND32_MTXSZ )
        mt_srand32(obj, DEFSEED32);

    // Calculate the twisted feedback

    r   = ( *obj->curr & MASKU32 ) | ( *obj->next & MASKL32 );
    *obj->curr = *obj->mm ^ ( r >> 1 ) ^ mat[ r & 0x01 ];  // Add MATRIX-A term if 'r' is an odd number

    // Calculate the main random number

    r  = *obj->curr;
    r ^= ( r >>  1 );
    r ^= ( r <<  7 ) & 0x9D2C5680;
    r ^= ( r << 15 ) & 0xEFC60000;
    r ^= ( r >> 18 );

    // Move points to next item

    ++obj->curr;
    if( obj->curr == obj->end ) obj->curr = obj->state;

    ++obj->next;
    if( obj->next == obj->end ) obj->next = obj->state;

    ++obj->mm;
    if( obj->mm == obj->end ) obj->mm = obj->state;

    return r;
}
//------------------------------------------------------------------------------
void mt_srand64(mt_rand64_t* /*optional*/ RESTRICT obj, uint64_t seed)
{
    /**
     * @brief Set random seed for 64-bits random number generator.
     * @param obj  Generator object.
     *             This parameter can be NULL if no generator object assigned,
     *             and a common object will be used automatically.
     * @param seed The random seed to initialize the generator.
     */
    int i;

    if( !obj ) obj = &shrd64;  // 若使用者不傳入指定的物件則使用預設的全域物件

    obj->state[0] = seed;
    for(i=1; i<MTRND64_MTXSZ; ++i)
        obj->state[i] = 0x5851F42D4C957F2D * ( obj->state[i-1] ^ ( obj->state[i-1] >> 62 ) ) + i;

    obj->end  = obj->state + MTRND64_MTXSZ;
    obj->curr = obj->state;
    obj->next = obj->state + 1;
    obj->mm   = obj->state + MM64;
}
//------------------------------------------------------------------------------
uint64_t mt_rand64(mt_rand64_t* /*optional*/ RESTRICT obj)
{
    /**
     * @brief Generate 64-bits random number.
     * @param obj Generator object.
     *            This parameter can be NULL if no generator object assigned,
     *            and a common object will be used automatically.
     * @return The random number generated.
     */
    static const uint64_t mat[2] = { 0, MATRIX_A64 };
    uint64_t r;

    if( !obj ) obj = &shrd64;  // 若使用者不傳入指定的物件則使用預設的全域物件

    // Initialize object if it is not initailized

    if( obj->end != obj->state + MTRND64_MTXSZ )
        mt_srand64(obj, DEFSEED64);

    // Calculate the twisted feedback

    r   = ( *obj->curr & MASKU64 ) | ( *obj->next & MASKL64 );
    *obj->curr = *obj->mm ^ ( r >> 1 ) ^ mat[ r & 0x01 ];  // Add MATRIX-A term if 'r' is an odd number

    // Calculate the main random number

    r  = *obj->curr;
    r ^= ( r >> 29 ) & 0x5555555555555555;
    r ^= ( r << 17 ) & 0x71D67FFFEDA60000;
    r ^= ( r << 37 ) & 0xFFF7EEE000000000;
    r ^= ( r >> 43 );

    // Move points to next item

    ++obj->curr;
    if( obj->curr == obj->end ) obj->curr = obj->state;

    ++obj->next;
    if( obj->next == obj->end ) obj->next = obj->state;

    ++obj->mm;
    if( obj->mm == obj->end ) obj->mm = obj->state;

    return r;
}
//------------------------------------------------------------------------------
void mt_randbin(mt_rand64_t* /*optional*/ RESTRICT obj, void* RESTRICT dest, size_t size)
{
    /**
     * Generate a set of random data.
     * @param obj  Generator object.
     *             This parameter can be NULL if no generator object assigned,
     *             and a common object will be used automatically.
     * @param dest A buffer to receive the result data.
     * @param size Size of the output buffer.
     */
    uint8_t  *buf = dest;
    uint64_t  rndval;

    if( !obj ) obj = &shrd64;  // 若使用者不傳入指定的物件則使用預設的全域物件
    if( !dest ) return;

    rndval = mt_rand64(obj);
    while( size )
    {
        if( !( size-- & 0x07 ) ) rndval = mt_rand64(obj);
        *buf++   = rndval & 0xFF;
        rndval >>= 8;
    }
}
//------------------------------------------------------------------------------

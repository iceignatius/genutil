/**
 * @file
 * @brief     Hash.
 * @details   To support simple hashing functions.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_HASH_H_
#define _GEN_HASH_H_

#include "../type.h"
#include "../restrict.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t  hash_pearson    (const void* RESTRICT src, size_t size);
uint16_t hash_crc16_ccitt(const void* RESTRICT src, size_t size);
uint16_t hash_crc16_ibm  (const void* RESTRICT src, size_t size);
uint32_t hash_crc32      (const void* RESTRICT src, size_t size);
uint32_t hash_jenkins    (const void* RESTRICT src, size_t size);
uint32_t hash_fnv32      (const void* RESTRICT src, size_t size);
uint32_t hash_murmur3_32 (const void* RESTRICT src, size_t size, uint32_t seed);
uint64_t hash_murmur3_64 (const void* RESTRICT src, size_t size, uint32_t seed);
void     hash_murmur3_128(byte_t dest[16], const void* RESTRICT src, size_t size, uint32_t seed);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

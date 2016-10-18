/**
 * @file
 * @brief     SHA.
 * @details   SHA hashing functions.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_SHA_H_
#define _GEN_SHA_H_

#ifdef __cplusplus
extern "C" {
#endif

void sha_1  (void *dest, const void *src, size_t size);
void sha_224(void *dest, const void *src, size_t size);
void sha_256(void *dest, const void *src, size_t size);
void sha_384(void *dest, const void *src, size_t size);
void sha_512(void *dest, const void *src, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

/**
 * @file
 * @brief     MD5.
 * @details   MD5 hashing functions.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_MD5_H_
#define _GEN_MD5_H_

#ifdef __cplusplus
extern "C" {
#endif

void md5(void *dest, const void *src, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

/******************************************************************************
 * Name      : RSA
 * Purpose   : RSA cipher functions.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _GEN_RSA_H_
#define _GEN_RSA_H_

#include "../type.h"
#include "../inline.h"
#include "../mtrand.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RSA_KEY_PARAMSZ 128  // Size of each RSA key parameter

typedef enum rsa_type_t
{
    rsa_type_128,
    rsa_type_256,
    rsa_type_512,
    rsa_type_768,
    rsa_type_1024,
} rsa_type_t;

typedef struct rsa_key_t
{
    byte_t modulus [RSA_KEY_PARAMSZ];  // The binary data of an integer with Little-Endian format
    byte_t exponent[RSA_KEY_PARAMSZ];  // The binary data of an integer with Little-Endian format
} rsa_key_t;

INLINE void rsa_generator_init(mt_rand64_t *rndobj, uint64_t seed){ mt_srand64(rndobj, seed); }
void rsa_generate_keys(rsa_key_t   /*out*/         *publickey,
                       rsa_key_t   /*out*/         *privatekey,
                       rsa_type_t  /*in */          type,
                       mt_rand64_t /*in,optional*/ *rndobj);

size_t rsa_get_state_size(rsa_type_t type);

size_t rsa_state_encipher(void            /*out*/ *out,
                          size_t          /*in */  outsz,
                          const void      /*in */ *in,
                          size_t          /*in */  insz,
                          const rsa_key_t /*in */ *key);
size_t rsa_state_decipher(void            /*out*/ *out,
                          size_t          /*in */  outsz,
                          const void      /*in */ *in,
                          size_t          /*in */  insz,
                          const rsa_key_t /*in */ *key);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

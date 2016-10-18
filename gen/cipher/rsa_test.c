#include <assert.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../bignum/const.h"
#include "../bignum/io.h"
#include "../bignum/math.h"
#include "rsa.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

void print_key(const char *label, const rsa_key_t *key)
{
    uintbig_t modulus, exponent;
    char      bufmod[2048], bufexp[2048];

    assert( label && key );

    modulus  = uintbig_from_bin(key->modulus , RSA_KEY_PARAMSZ);
    exponent = uintbig_from_bin(key->exponent, RSA_KEY_PARAMSZ);
    uintbig_to_str(bufmod, sizeof(bufmod), &modulus);
    uintbig_to_str(bufexp, sizeof(bufexp), &exponent);
    printf("%s :\n  modulus  = %s\n  exponent = %s\n", label, bufmod, bufexp);
}

int main(void)
{
    byte_t     text0[1024];
    byte_t     code [1024];
    byte_t     text1[1024];
    size_t     ntext0;
    size_t     ncode;
    size_t     ntext1;
    int        i;
    rsa_key_t  publickey, privatekey;
    rsa_type_t rsatype;
    rsa_type_t rsatype_table[] = {
                                   rsa_type_128,
                                   rsa_type_256,
                                   rsa_type_512,
                                   rsa_type_768,
                                   rsa_type_1024,
                                 };

    rsa_generator_init(NULL, time(NULL));

    for(i=0; i<sizeof(rsatype_table)/sizeof(rsatype_table[0]); ++i)
    {
        rsatype = rsatype_table[i];
        ntext0  = rsa_get_state_size(rsatype) - 1;
        mt_randbin(NULL, text0, ntext0);

        rsa_generate_keys(&publickey, &privatekey, rsatype, NULL);
        print_key("public ", &publickey);
        print_key("private", &privatekey);

        assert( ncode  = rsa_state_encipher(code , sizeof(code), text0, ntext0, &publickey ) );
        assert( ntext1 = rsa_state_decipher(text1, ntext0      , code , ncode , &privatekey) );
        assert( ntext1 == ntext0 );
        assert( 0 == memcmp(text1, text0, ntext0) );
    }

    return 0;
}

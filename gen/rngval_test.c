#include <assert.h>
#include "rngval.h"

int main(int argc, char *argv[])
{
    char str[] = "1-3,5,2-4,7";

    rngval_t rng;
    int      val;

    assert( 1 == ( val = rngval_get_first(&rng, str, 10) ) );
    assert( 2 == ( val = rngval_get_next(&rng, val) ) );
    assert( 3 == ( val = rngval_get_next(&rng, val) ) );
    assert( 5 == ( val = rngval_get_next(&rng, val) ) );
    assert( 2 == ( val = rngval_get_next(&rng, val) ) );
    assert( 3 == ( val = rngval_get_next(&rng, val) ) );
    assert( 4 == ( val = rngval_get_next(&rng, val) ) );
    assert( 7 == ( val = rngval_get_next(&rng, val) ) );
    assert( 0 >  ( val = rngval_get_next(&rng, val) ) );

    return 0;
}

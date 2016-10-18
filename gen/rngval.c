#include <stdlib.h>
#include <string.h>
#include "minmax.h"
#include "rngval.h"

//------------------------------------------------------------------------------
int rngval_get_first(rngval_t *rng, const char *str, int radix)
{
    /**
     * Get first value from a value range string.
     *
     * @param[out] rng   It will return the status record of range analysis.
     * @param[in]  str   A string in range value string format to parse.
     * @param[in]  radix Numerical base that determines the valid characters and their interpretation.
     *                   And can be ZERO to auto detect.
     * @return A positive value (including zero) is the first value of the string; or
     *         a negative value indicates that there have no values rest or
     *         the format of string does not correct.
     */
    rng->pos   = str;
    rng->top   = -1;
    rng->radix = radix;
    return rngval_get_next(rng, -1);
}
//------------------------------------------------------------------------------
static
int str_to_int(const char *str, int radix)
{
    char *end;
    int val = strtol(str, &end, radix);
    return end != str ? val : -1;
}
//------------------------------------------------------------------------------
static
const char* get_first_comma_separated_str(char *buf, size_t size, const char *str)
{
    memset(buf, 0, size);

    const char *begin  = str;
    if( !begin ) return NULL;

    const char *end = strchr(begin, ',');
    str = end ? end + 1 : NULL;

    size_t len = end ? end - begin : strlen(begin);
    if( len )
        memcpy(buf, begin, MIN(len,size));

    return str;
}
//------------------------------------------------------------------------------
static
void dash_str_get_lval_and_rval(char *str, int radix, int *lval, int *rval)
{
    char *dash = strchr(str, '-');
    char *lstr = str;
    char *rstr = dash ? dash + 1 : str;
    if( dash ) *dash = 0;

    *lval = str_to_int(lstr, radix);
    *rval = str_to_int(rstr, radix);
}
//------------------------------------------------------------------------------
int rngval_get_next(rngval_t *rng, int prevval)
{
    /**
     * Get next value from a value range string.
     *
     * @param[in,out] rng
     * @param         prevval
     * @return A positive value (including zero) is the next value of the string; or
     *         a negative value indicates that there have no values rest.
     *
     * @remarks
     *      @li This function will try to skip characters with
     *          incorrect format in the string.
     *      @li This function does not guarantee that
     *          the result value will ordered in any order or
     *          will not be duplicated.
     */
    if( prevval >= 0 && prevval < rng->top ) return ++prevval;

    rng->top = -1;
    if( !rng->pos ) return -1;

    char str[ strlen(rng->pos) + 1 ];
    rng->pos = get_first_comma_separated_str(str, sizeof(str), rng->pos);

    int val;
    dash_str_get_lval_and_rval(str, rng->radix, &val, &rng->top);
    if( rng->pos >= 0 && val < 0 )
        rng->top = -1;

    return val >= 0 ? val : prevval;
}
//------------------------------------------------------------------------------

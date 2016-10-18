/**
 * @file
 * @brief     Range value string parser.
 * @details   A parser to parse number values from a formatted string.
 * @author    王文佑
 * @date      2015.10.22
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
/**
 * @file
 * @remarks The format of range value string is digit characters separated with comma and dash.
 *          A comma separated two values or value ranges, and
 *          a dash declared that a set of values from lower bound to higher bound.
 *          For example, a string of "2-5,7,9" will be parsed as values: 2, 3, 4, 5, 7, and 9.
 */
#ifndef _GEN_RNGVAL_H_
#define _GEN_RNGVAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Value range string parse status.
 */
typedef struct rngval_t
{
    const char *pos;
    int         top;
    int         radix;
} rngval_t;

int rngval_get_first(rngval_t *rng, const char *str, int radix);
int rngval_get_next(rngval_t *rng, int prevval);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

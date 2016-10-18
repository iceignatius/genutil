/**
 * @file
 * @brief     Minimum and maximum.
 * @details   Functions or macros to get minimum and maximum value, especially for C file.
 * @author    王文佑
 * @date      2014.10.02
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_MINMAX_H_
#define _GEN_MINMAX_H_

/// Get minimum value.
#ifndef MIN
#define MIN(a,b) (( (a) < (b) )?( a ):( b ))
#endif

/// Get maximum value.
#ifndef MAX
#define MAX(a,b) (( (a) > (b) )?( a ):( b ))
#endif

#endif

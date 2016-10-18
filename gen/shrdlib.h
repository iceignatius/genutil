/**
 * @file
 * @brief     Shared library loader.
 * @details   Encapsulation of shared library loader.
 * @author    王文佑
 * @date      2014.01.25
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GNE_SHRDLIB_H_
#define _GEN_SHRDLIB_H_

#include <stdbool.h>
#include "restrict.h"
#include "shrdlibdef.h"

/**
 * @file
 * @note User will need to link "dl" library (-ldl) in Linux.
 */

/// Library handler.
typedef void* shrdlib_t;

shrdlib_t shrdlib_open   (const char* RESTRICT filename);
bool      shrdlib_close  (shrdlib_t* RESTRICT handle);
void*     shrdlib_getfunc(shrdlib_t* RESTRICT handle, const char* RESTRICT funcname);

#endif

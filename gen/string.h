/**
 * @file
 * @brief     String extension tools.
 * @details   More tools about string operation.
 * @author    王文佑
 * @date      2014.10.02
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_STRING_H_
#define _GEN_STRING_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

char* str_trim_trailing_space(char *str);
char* str_trim_leading_space (char *str);
char* str_escch_to_ctrlcode  (char *str);
char* str_paste(char /*in,out*/ *dest, size_t size, const char *src, size_t offset);

size_t str_bin_to_hexstr(char       *dest,
                         size_t      destsz,
                         const void *data,
                         size_t      datsz,
                         const char *prefix,
                         const char *postfix);
size_t str_hexstr_to_bin(void       *dest,
                         size_t      destsz,
                         const char *str,
                         bool        skip_errch);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

/**
 * @file
 * @brief     Time information
 * @details   Translate between date time digits and UNIX timestamps.
 * @author    王文佑
 * @date      2014.05.25
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 *
 * @note Unix Timestamp: Seconds passed from 1970/01/01 00:00:00, and can be a negative value.
 */
#ifndef _GEN_TIMEINF_H_
#define _GEN_TIMEINF_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time information in digits
 */
typedef struct timeinf_t
{
    unsigned year;    ///< Years in Common Era (C.E.) or Anno Domini (A.D.).
    unsigned month;   ///< Range : 1 ~ 12
    unsigned day;     ///< Range : 1 ~ 31
    unsigned hour;    ///< Range : 0 ~ 23
    unsigned minute;  ///< Range : 0 ~ 59
    unsigned second;  ///< Range : 0 ~ 59
} timeinf_t;

timeinf_t timeinf_from_uxtime(long long uxtime);
long long timeinf_to_uxtime  (const timeinf_t *timeinf);
bool      timeinf_is_equal   (const timeinf_t *timeinf1, const timeinf_t *timeinf2);

char* timeinf_to_datestr(const timeinf_t *timeinf, char *buf, size_t bufsz, const char *delimiter);
char* timeinf_to_timestr(const timeinf_t *timeinf, char *buf, size_t bufsz, const char *delimiter);

char* timeinf_to_datetimestr(const timeinf_t *timeinf,
                             char            *buf,
                             size_t           bufsz,
                             const char      *datedeli,
                             const char      *timedeli,
                             const char      *middeli);

unsigned timeinf_get_day_of_year(const timeinf_t *timeinf);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

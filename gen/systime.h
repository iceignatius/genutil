/**
 * @file
 * @brief     System time.
 * @details   Encapsulation of functions about system time.
 * @author    王文佑
 * @date      2014.03.09
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 *
 * @note Unix Timestamp: Seconds passed from 1970/01/01 00:00:00, and can be a negative value.
 */
#ifndef _GEN_SYSTIME_H_
#define _GEN_SYSTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
    #define SYSTIME_DEPRECATED __attribute__ ((deprecated))
#else
    define SYSTIME_DEPRECATED
#endif

void      systime_sleep(unsigned ms);
void      systime_sleep_fragment(void) SYSTIME_DEPRECATED;
void      systime_sleep_awhile(void);
unsigned  systime_get_clock_count(void);

int       systime_get_timezone(void);
long long systime_get_utc(void);
long long systime_get_local(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

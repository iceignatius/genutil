/**
 * @file
 * @brief     Time counter
 * @details   Encapsulate time count operations to help user to implement some timer behaviour.
 * @author    王文佑
 * @date      2015.07.23
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_TIMECTR_H_
#define _GEN_TIMECTR_H_

#include <stdbool.h>
#include "inline.h"
#include "systime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @class timectr_t
 * @brief Time counter.
 */
typedef struct timectr_t
{
    unsigned start;
    unsigned range;
} timectr_t;

INLINE
void timectr_reset(timectr_t *ctr, unsigned timeout)
{
    /**
     * @memberof timectr_t
     * @brief Reset time counter.
     *
     * @param ctr     Object instance.
     * @param timeout The time out value in milliseconds.
     */
    ctr->start = systime_get_clock_count();
    ctr->range = timeout;
}

INLINE
void timectr_reset_ctronly(timectr_t *ctr)
{
    /**
     * @memberof timectr_t
     * @brief Reset time counter with the current time out setting.
     */
    ctr->start = systime_get_clock_count();
}

INLINE
void timectr_init(timectr_t *ctr, unsigned timeout)
{
    /**
     * @memberof timectr_t
     * @brief Initialize time counter.
     * @details This function is actually the alias of timectr_t::timectr_reset
     *          to help user make their code more readability.
     */
    timectr_reset(ctr, timeout);
}

INLINE
timectr_t timectr_init_inline(unsigned timeout)
{
    /**
     * @memberof timectr_t
     * @brief Initialize time counter.
     * @details This function actually do the same thing of timectr_t::timectr_init,
     *          but try to help user to decrease their code lines.
     */
    timectr_t ctr;
    timectr_init(&ctr, timeout);
    return ctr;
}

INLINE
unsigned timectr_get_timeout(const timectr_t *ctr)
{
    /**
     * @memberof timectr_t
     * @brief Get time out setting of the time counter.
     *
     * @param ctr Object instance.
     * @return The time out value in milliseconds.
     */
    return ctr->range;
}

INLINE
unsigned timectr_get_passed(const timectr_t *ctr)
{
    /**
     * @memberof timectr_t
     * @brief Get the time passed from the last time the time counter been reset.
     *
     * @param ctr Object instance.
     * @return The time passed in milliseconds.
     */
    return systime_get_clock_count() - ctr->start;
}

INLINE
unsigned timectr_get_remain(const timectr_t *ctr)
{
    /**
     * @memberof timectr_t
     * @brief Get time remain.
     *
     * @param ctr Object instance.
     * @return The time remain in milliseconds;
     *         and ZERO will be returned if the time has expired.
     */
    unsigned passed = systime_get_clock_count() - ctr->start;
    return ( passed < ctr->range )?( ctr->range - passed ):( 0 );
}

INLINE
bool timectr_is_expired(const timectr_t *ctr)
{
    /**
     * @memberof timectr_t
     * @brief Check if the timer expired.
     *
     * @param ctr Object instance.
     * @return TRUE if the timer has expired
     *         (the time passed be equal or longer then the time out value);
     *         and FALSE if not.
     */
    return systime_get_clock_count() - ctr->start >= ctr->range;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/**
 * @brief C++ wrapper of @ref timectr_t
 */
class TTimeCounter : protected timectr_t
{
public:
    TTimeCounter(unsigned timeout=0) { timectr_init(this, timeout); }  ///< @see timectr_t::timectr_init

public:
    void     Reset(unsigned timeout) {        timectr_reset(this, timeout); }  ///< @see timectr_t::timectr_reset
    void     Reset()                 {        timectr_reset_ctronly(this); }   ///< @see timectr_t::timectr_reset_cntonly
    unsigned GetTimeout()      const { return timectr_get_timeout(this); }     ///< @see timectr_t::timectr_get_timeout
    unsigned GetPassed()       const { return timectr_get_passed(this); }      ///< @see timectr_t::timectr_get_passed
    unsigned GetRemain()       const { return timectr_get_remain(this); }      ///< @see timectr_t::timectr_get_remain
    bool     IsExpired()       const { return timectr_is_expired(this); }      ///< @see timectr_t::timectr_is_expired

};

#endif

#endif

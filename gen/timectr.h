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

static inline
timectr_t timectr_init(timectr_t *self, unsigned timeout)
{
    /**
     * @memberof timectr_t
     * @brief Initialise time counter.
     *
     * @param self    The buffer to a counter object to be initialised,
     *                and can be NULL if you want assign the initial value by the return value.
     * @param timeout The time out value in milliseconds.
     * @return The initialised object.
     */
    timectr_t inst;
    self = self ? self : &inst;

    self->start = systime_get_clock_count();
    self->range = timeout;

    return *self;
}

static inline
void timectr_reset_all(timectr_t *self, unsigned timeout)
{
    /**
     * @memberof timectr_t
     * @brief Reset time counter and all its properties.
     *
     * @param self    Object instance.
     * @param timeout The time out value in milliseconds.
     */
    self->start = systime_get_clock_count();
    self->range = timeout;
}

static inline
void timectr_reset(timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Restart time count from now.
     */
    self->start = systime_get_clock_count();
}

static inline
void timectr_next_round(timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Restart time count from the last expired time.
     */
    self->start += self->range;
}

static inline
unsigned timectr_get_timeout(const timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Get time out setting of the time counter.
     *
     * @param self Object instance.
     * @return The time out value in milliseconds.
     */
    return self->range;
}

static inline
unsigned timectr_get_passed(const timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Get the time passed from the last time the time counter been reset.
     *
     * @param self Object instance.
     * @return The time passed in milliseconds.
     */
    return systime_get_clock_count() - self->start;
}

static inline
unsigned timectr_get_remain(const timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Get time remain.
     *
     * @param self Object instance.
     * @return The time remain in milliseconds;
     *         and ZERO will be returned if the time has expired.
     */
    unsigned passed = systime_get_clock_count() - self->start;
    return ( passed < self->range )?( self->range - passed ):( 0 );
}

static inline
bool timectr_is_expired(const timectr_t *self)
{
    /**
     * @memberof timectr_t
     * @brief Check if the timer expired.
     *
     * @param self Object instance.
     * @return TRUE if the timer has expired
     *         (the time passed be equal or longer then the time out value);
     *         and FALSE if not.
     */
    return systime_get_clock_count() - self->start >= self->range;
}

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/**
 * @brief C++ wrapper of @ref timectr_t
 */
class TimeCounter : protected timectr_t
{
public:
    TimeCounter(unsigned timeout=0) { timectr_init(this, timeout); }  ///< @see timectr_t::timectr_init

public:
    void     Reset(unsigned timeout) {        timectr_reset_all(this, timeout); }   ///< @see timectr_t::timectr_reset_all
    void     Reset()                 {        timectr_reset(this); }                ///< @see timectr_t::timectr_reset
    void     NextRound()             {        timectr_next_round(this); }           ///< @see timectr_t::timectr_next_round
    unsigned GetTimeout()      const { return timectr_get_timeout(this); }          ///< @see timectr_t::timectr_get_timeout
    unsigned GetPassed()       const { return timectr_get_passed(this); }           ///< @see timectr_t::timectr_get_passed
    unsigned GetRemain()       const { return timectr_get_remain(this); }           ///< @see timectr_t::timectr_get_remain
    bool     IsExpired()       const { return timectr_is_expired(this); }           ///< @see timectr_t::timectr_is_expired

};

#endif

#endif

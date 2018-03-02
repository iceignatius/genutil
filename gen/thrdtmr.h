/**
 * @file
 * @brief     Thread timer
 * @details   Timer worked with an separated thread.
 * @author    王文佑
 * @date      2014.10.27
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_THRDTMR_H_
#define _GEN_THRDTMR_H_

#include <stdbool.h>

#ifdef __cplusplus
#include <new>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @memberof thrdtmr_t
 * Callback function when timer starting up.
 * User can use this to initialize or start up some resources.
 *
 * @param userarg An user defined argument.
 * @return An user defined error code, and MUST be ZERO if function succeed.
 *
 * @remarks The return code other than zero will cause the timer go terminate,
 *          and that value will be passed to the caller who want start the timer.
 */
typedef int(*thrdtmr_on_start_t)(void *userarg);

/**
 * @memberof thrdtmr_t
 * Callback function when the timer event triggered.
 *
 * @param userarg An user defined argument.
 */
typedef void(*thrdtmr_on_timer_t)(void *userarg);

/**
 * @memberof thrdtmr_t
 * Callback function when timer going terminating.
 * User can use this to end and release some resources, and report result.
 *
 * @param userarg An user defined argument.
 * @return An user defined return code, but we recommended to return ZERO for normal cases.
 *
 * @remarks This function will be called when timer going terminating,
 *          even if ::thrdtmr_on_start_t callback failed.
 */
typedef int(*thrdtmr_on_stop_t)(void *userarg);

/**
 * @class thrdtmr_t
 * @brief Thread timer
 */
typedef struct thrdtmr_t thrdtmr_t;

thrdtmr_t* thrdtmr_create(unsigned            interval,
                          void               *userarg,
                          thrdtmr_on_start_t  on_start,
                          thrdtmr_on_timer_t  on_timer,
                          thrdtmr_on_stop_t   on_stop);
void thrdtmr_release(thrdtmr_t *self);

unsigned thrdtmr_get_interval(const thrdtmr_t *self);
void     thrdtmr_set_interval(      thrdtmr_t *self, unsigned value);

int thrdtmr_start(thrdtmr_t *self);
int thrdtmr_stop (thrdtmr_t *self, bool wait_stopped);

bool thrdtmr_is_started(const thrdtmr_t *self);

/// @memberof thrdtmr_t
/// @brief Terminate the timer and wait until it is terminated.
/// @see thrdtmr_t::thrdtmr_stop
static inline
int thrdtmr_stop_and_wait(thrdtmr_t *self)
{
    return thrdtmr_stop(self, true);
}

/// @memberof thrdtmr_t
/// @brief Terminate the timer without waiting.
/// @see thrdtmr_t::thrdtmr_stop
static inline
void thrdtmr_stop_not_wait(thrdtmr_t *self)
{
    thrdtmr_stop(self, false);
}

#ifdef __cplusplus
}  // extern "C"
#endif

// C++ Wrapper
#ifdef __cplusplus

/**
 * @brief C++ wrapper of @ref thrdtmr_t
 */
class ThreadTimer
{
private:
    thrdtmr_t *timer;

private:
    static int  CThrdOnStart(ThreadTimer *self) { return self->OnStart(); }
    static void CThrdOnTimer(ThreadTimer *self) {        self->OnTimer(); }
    static int  CThrdOnStop (ThreadTimer *self) { return self->OnStop(); }

public:
    ThreadTimer(unsigned Interval=0)
    {
        /// @see thrdtmr_t::thrdtmr_init
        timer = thrdtmr_create(Interval,
                               this,
                               (int(*)(void*))  CThrdOnStart,
                               (void(*)(void*)) CThrdOnTimer,
                               (int(*)(void*))  CThrdOnStop);
        if( !timer ) throw std::bad_alloc();
    }

    virtual ~ThreadTimer()
    {
        thrdtmr_release(timer);
    }

private:
    ThreadTimer(const ThreadTimer &Src);             // Not allowed to use
    ThreadTimer& operator=(const ThreadTimer &Src);  // Not allowed to use

protected:
    virtual int  OnStart() { return 0; }    ///< @see thrdtmr_t::thrdtmr_on_start_t
    virtual void OnTimer() =0;              ///< @see thrdtmr_t::thrdtmr_on_timer_t
    virtual int  OnStop() { return 0; }     ///< @see thrdtmr_t::thrdtmr_on_stop_t

public:
    unsigned GetInterval()         const { return thrdtmr_get_interval (timer); }           ///< @see thrdtmr_t::thrdtmr_get_interval
    void     SetInterval(unsigned value) {        thrdtmr_set_interval (timer, value); }    ///< @see thrdtmr_t::thrdtmr_set_interval
    int      Start()                     { return thrdtmr_start        (timer); }           ///< @see thrdtmr_t::thrdtmr_start
    bool     IsStarted()           const { return thrdtmr_is_started   (timer); }           ///< @see thrdtmr_t::thrdtmr_is_started
    int      StopAndWait()               { return thrdtmr_stop_and_wait(timer); }           ///< @see thrdtmr_t::thrdtmr_stop_and_wait
    void     StopNotWait()               {        thrdtmr_stop_not_wait(timer); }           ///< @see thrdtmr_t::thrdtmr_stop_not_wait

};

#endif

#endif

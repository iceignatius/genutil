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
#include <threads.h>
#include "inline.h"

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
typedef int(*thrdtmr_on_startup_t)(void *userarg);

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
 *          even if ::thrdtmr_on_startup_t callback failed.
 */
typedef int(*thrdtmr_on_terminating_t)(void *userarg);

/**
 * @class thrdtmr_t
 * @brief Thread timer
 */
typedef struct thrdtmr_t
{
    // WARNING: All variables are private!

    // Thread and timer
    mtx_t    terminate_mutex;
    thrd_t   thread;
    bool     thread_available;
    unsigned interval;      // Time interval in milliseconds.
    bool     go_terminate;  // Flag to request thread go terminate.
    int      status;

    // Callbacks
    void                     *userarg;
    thrdtmr_on_startup_t      on_startup;
    thrdtmr_on_timer_t        on_timer;
    thrdtmr_on_terminating_t  on_terminating;

} thrdtmr_t;

void     thrdtmr_init         (      thrdtmr_t *timer, unsigned                 interval,
                                                       void                    *userarg,
                                                       thrdtmr_on_startup_t     on_startup,
                                                       thrdtmr_on_timer_t       on_timer,
                                                       thrdtmr_on_terminating_t on_terminating);
void     thrdtmr_deinit       (      thrdtmr_t *timer);
unsigned thrdtmr_get_interval (const thrdtmr_t *timer);
void     thrdtmr_set_interval (      thrdtmr_t *timer, unsigned value);
int      thrdtmr_start        (      thrdtmr_t *timer);
int      thrdtmr_terminate    (      thrdtmr_t *timer, bool wait_terminated);
bool     thrdtmr_is_terminated(const thrdtmr_t *timer);

/// @memberof thrdtmr_t
/// @brief Terminate the timer and wait until it is terminated.
/// @see thrdtmr_t::thrdtmr_terminate
INLINE int thrdtmr_terminate_and_wait_terminated(thrdtmr_t *timer) { return thrdtmr_terminate(timer, true); }

/// @memberof thrdtmr_t
/// @brief Terminate the timer without waiting.
/// @see thrdtmr_t::thrdtmr_terminate
INLINE void thrdtmr_terminate_without_waiting(thrdtmr_t *timer) {thrdtmr_terminate(timer, false); }

#ifdef __cplusplus
}  // extern "C"
#endif

// C++ Wrapper
#ifdef __cplusplus

/**
 * @brief C++ wrapper of @ref thrdtmr_t
 */
class TThreadTimer
{
private:
    thrdtmr_t Timer;

private:
    static int  CThrdOnStartup    (TThreadTimer *This) { return This->OnStartup(); }
    static void CThrdOnTimer      (TThreadTimer *This) {        This->OnTimer(); }
    static int  CThrdOnTerminating(TThreadTimer *This) { return This->OnTerminating(); }

public:
    TThreadTimer(unsigned Interval=0)
    {
        /// @see thrdtmr_t::thrdtmr_init
        thrdtmr_init(&Timer,
                     Interval,
                     this,
                     (thrdtmr_on_startup_t)    CThrdOnStartup,
                     (thrdtmr_on_timer_t)      CThrdOnTimer,
                     (thrdtmr_on_terminating_t)CThrdOnTerminating);
    }

    ~TThreadTimer()
    {
        thrdtmr_deinit(&Timer);
    }

private:
    TThreadTimer(const TThreadTimer &Src);             // Not allowed to use
    TThreadTimer& operator=(const TThreadTimer &Src);  // Not allowed to use

protected:
    virtual int  OnStartup() { return 0; }      ///< @see thrdtmr_t::thrdtmr_on_startup_t
    virtual void OnTimer() =0;                  ///< @see thrdtmr_t::thrdtmr_on_timer_t
    virtual int  OnTerminating() { return 0; }  ///< @see thrdtmr_t::thrdtmr_on_terminating_t

public:
    unsigned GetInterval()                const { return thrdtmr_get_interval (&Timer); }         ///< @see thrdtmr_t::thrdtmr_get_interval
    void     SetInterval(unsigned Value)        {        thrdtmr_set_interval (&Timer, Value); }  ///< @see thrdtmr_t::thrdtmr_set_interval
    int      Start()                            { return thrdtmr_start        (&Timer); }         ///< @see thrdtmr_t::thrdtmr_start
    bool     IsTerminated()               const { return thrdtmr_is_terminated(&Timer); }         ///< @see thrdtmr_t::thrdtmr_is_terminated
    int      TerminateAndWaitTerminated()       { return thrdtmr_terminate    (&Timer, true); }   ///< @see thrdtmr_t::thrdtmr_terminate_and_wait_terminated
    void     TerminateWithoutWaiting()          {        thrdtmr_terminate    (&Timer, false); }  ///< @see thrdtmr_t::thrdtmr_terminate_without_waiting

};

#endif

#endif

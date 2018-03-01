#include <assert.h>
#include <stdlib.h>
#include "systime.h"
#include "thrdtmr.h"

enum
{
    TMS_TERMINATED,
    TMS_INITIALISED,
    TMS_START_CHECKED,
    TMS_LOOPING,
    TMS_FINISHING,
};

//------------------------------------------------------------------------------
static
int on_startup_default(void *arg)
{
    return 0;
}
//------------------------------------------------------------------------------
static
void on_timer_default(void *arg)
{
}
//------------------------------------------------------------------------------
static
int on_terminating_default(void *arg)
{
    return 0;
}
//------------------------------------------------------------------------------
void thrdtmr_init(thrdtmr_t               *timer,
                  unsigned                 interval,
                  void                    *userarg,
                  thrdtmr_on_startup_t     on_startup,
                  thrdtmr_on_timer_t       on_timer,
                  thrdtmr_on_terminating_t on_terminating)
{
    /**
     * @memberof thrdtmr_t
     * @brief Constructor.
     *
     * @param timer          Object instance.
     * @param interval       Timer interval in milliseconds.
     * @param userarg        A user defined parameter to passed to all callbacks,
     *                       and can be NULL if it is no need to use.
     * @param on_startup     A callback function that will be called when timer thread starting up,
     *                       and can be NULL if a callback is not needed.
     * @param on_timer       A callback function that will be called on each timer event,
     *                       and can be NULL if a callback is not needed.
     * @param on_terminating A callback function that will be called when timer thread going terminating,
     *                       and can be NULL if a callback is not needed.
     */
    timer->thread_available = false;

    mtx_init(&timer->terminate_mutex, mtx_plain);
    timer->go_terminate = false;
    timer->status       = TMS_TERMINATED;

    timer->interval = interval;

    timer->userarg = userarg;
    timer->on_startup     = on_startup     ? on_startup     : on_startup_default;
    timer->on_timer       = on_timer       ? on_timer       : on_timer_default;
    timer->on_terminating = on_terminating ? on_terminating : on_terminating_default;
}
//------------------------------------------------------------------------------
void thrdtmr_deinit(thrdtmr_t *timer)
{
    /**
     * @memberof thrdtmr_t
     * @brief Destructor.
     *
     * @param timer Object instance.
     */
    thrdtmr_terminate_and_wait_terminated(timer);
    mtx_destroy(&timer->terminate_mutex);
}
//------------------------------------------------------------------------------
thrdtmr_t* thrdtmr_create(unsigned                 interval,
                          void                    *userarg,
                          thrdtmr_on_startup_t     on_startup,
                          thrdtmr_on_timer_t       on_timer,
                          thrdtmr_on_terminating_t on_terminating)
{
    /**
     * @memberof thrdtmr_t
     * @brief Create a timer object.
     *
     * @param interval       Timer interval in milliseconds.
     * @param userarg        A user defined parameter to passed to all callbacks,
     *                       and can be NULL if it is no need to use.
     * @param on_startup     A callback function that will be called when timer thread starting up,
     *                       and can be NULL if a callback is not needed.
     * @param on_timer       A callback function that will be called on each timer event,
     *                       and can be NULL if a callback is not needed.
     * @param on_terminating A callback function that will be called when timer thread going terminating,
     *                       and can be NULL if a callback is not needed.
     * @return The new object instance, or NULL if failed!
     */
    thrdtmr_t *timer = malloc(sizeof(thrdtmr_t));

    if( timer )
        thrdtmr_init(timer, interval, userarg, on_startup, on_timer, on_terminating);

    return timer;
}
//------------------------------------------------------------------------------
void thrdtmr_release(thrdtmr_t *timer)
{
    /**
     * @memberof thrdtmr_t
     * @brief Release a timer object.
     *
     * @param timer Object instance.
     */
    if( timer )
    {
        thrdtmr_deinit(timer);
        free(timer);
    }
}
//------------------------------------------------------------------------------
unsigned thrdtmr_get_interval(const thrdtmr_t *timer)
{
    /**
     * @memberof thrdtmr_t
     * @brief Get timer interval.
     *
     * @param timer Object instance.
     * @return The timer interval in milliseconds.
     */
    return timer->interval;
}
//------------------------------------------------------------------------------
void thrdtmr_set_interval(thrdtmr_t *timer, unsigned value)
{
    /**
     * @memberof thrdtmr_t
     * @brief Set timer interval.
     *
     * @param timer Object instance.
     * @param value The time interval in milliseconds to set.
     */
    timer->interval = value;
}
//------------------------------------------------------------------------------
static
int THRDS_CALL thread_process(thrdtmr_t *timer)
{
    int retcode = 0;

    // Initialize
    timer->go_terminate = timer->on_startup(timer->userarg);
    timer->status       = TMS_INITIALISED;
    while( timer->status == TMS_INITIALISED ) systime_sleep_awhile();

    // Loop work
    if( !timer->go_terminate ) timer->status = TMS_LOOPING;
    unsigned time_prev = systime_get_clock_count();
    while( !timer->go_terminate )
    {
        if( timer->interval )
        {
            // Run in interval count mode
            if( systime_get_clock_count() - time_prev >= timer->interval )
            {
                timer->on_timer(timer->userarg);
                if( timer->go_terminate ) break;

                time_prev += timer->interval;
            }
        }
        else
        {
            // Run in interval ignored mode
            time_prev = systime_get_clock_count();
            timer->on_timer(timer->userarg);
        }

        // Idel for a while
        systime_sleep_awhile();
    }

    // Finalise
    timer->status = TMS_FINISHING;
    retcode = timer->on_terminating(timer->userarg);
    timer->status = TMS_TERMINATED;

    return retcode;
}
//------------------------------------------------------------------------------
int thrdtmr_start(thrdtmr_t *timer)
{
    /**
     * @memberof thrdtmr_t
     * @brief Start the timer.
     *
     * @param timer Object instance.
     * @return ZERO if succeed;
     *         or -1 if thread create failed;
     *         or other error code defined by user if user had set the on_starting callback.
     *
     * @remarks This function will be failed because of :
     *          @li The thread creation failure.
     *          @li The user defined callback (on_starting) returns failure result.
     */
    int retcode = 0;

    // Terminate the current thread
    thrdtmr_terminate_and_wait_terminated(timer);

    // Start thread
    timer->go_terminate = false;
    timer->thread_available =
        ( thrd_success == thrd_create(&timer->thread,
                                      (thrd_start_t) thread_process,
                                      timer) );
    if( !timer->thread_available ) return -1;

    // Wait thread be initialized or failed
    while( timer->status != TMS_INITIALISED ) systime_sleep_awhile();
    timer->status = TMS_START_CHECKED;

    // Wait thread be terminated if it initialize failed
    if( timer->go_terminate ) retcode = thrdtmr_terminate_and_wait_terminated(timer);

    return retcode;
}
//------------------------------------------------------------------------------
int thrdtmr_terminate(thrdtmr_t *timer, bool wait_terminated)
{
    /**
     * @memberof thrdtmr_t
     * @private
     * @brief Terminate the timer.
     *
     * @param timer           Object instance.
     * @param wait_terminated A flag to indecate if function should wait the timer terminated.
     *                        This function will return immediately if this flag set to FALSE.
     * @return The result returned by the user defined callback (on_ending);
     *         or ZERO by default.
     *
     * @note The return value will always be zero if @a wait_terminated set to FALSE.
     */
    int res = 0;

    assert(( timer->status != TMS_TERMINATED )||( !timer->thread_available ));
    if( timer->status == TMS_TERMINATED ) return 0;

    mtx_lock(&timer->terminate_mutex);
    if( timer->thread_available )
    {
        bool already_terminating =
            ( timer->status == TMS_FINISHING || timer->status == TMS_TERMINATED );

        if( !already_terminating )
        {
            timer->status = TMS_FINISHING;

            timer->go_terminate     = true;
            timer->thread_available = false;
            if( wait_terminated )
                thrd_join(timer->thread, &res);
            else
                thrd_detach(timer->thread);
        }
    }
    mtx_unlock(&timer->terminate_mutex);

    if( wait_terminated )
    {
        // Backup for the case that the thread has been detached earlier
        // but the thread has not finished.
        while( timer->status != TMS_TERMINATED )
            systime_sleep_awhile();
    }

    return res;
}
//------------------------------------------------------------------------------
bool thrdtmr_is_terminated(const thrdtmr_t *timer)
{
    /**
     * @memberof thrdtmr_t
     * @brief Check if the timer is terminated.
     *
     * @param timer  Object instance.
     * @return TRUE if the timer is terminated; and FALSE if not.
     */
    return timer->status == TMS_TERMINATED;
}
//------------------------------------------------------------------------------

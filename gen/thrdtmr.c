#include <stdatomic.h>
#include <stdlib.h>
#include <threads.h>
#include "systime.h"
#include "thrdtmr.h"

typedef atomic_int sync_t;

struct thrdtmr_t
{
    struct
    {
        bool   available;
        thrd_t instance;
    } thread;

    struct
    {
        int    res;
        sync_t sync;
    } up;

    struct
    {
        atomic_int go_terminate;    // Treat as boolean value.
    } down;

    unsigned interval;  // Time interval in milliseconds.

    struct
    {
        void *userarg;
        thrdtmr_on_start_t on_start;
        thrdtmr_on_timer_t on_timer;
        thrdtmr_on_stop_t  on_stop;
    } events;
};

//------------------------------------------------------------------------------
static
void sync_init(sync_t *self)
{
    atomic_init(self, 0);
}
//------------------------------------------------------------------------------
static
void sync_reset(sync_t *self)
{
    atomic_store(self, 2);
}
//------------------------------------------------------------------------------
static
void sync_wait(sync_t *self)
{
    atomic_fetch_sub(self, 1);

    while( atomic_load(self) > 0 )
        systime_sleep_awhile();
}
//------------------------------------------------------------------------------
static
int on_start_default(void *arg)
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
int on_stop_default(void *arg)
{
    return 0;
}
//------------------------------------------------------------------------------
static
void thrdtmr_init(thrdtmr_t          *self,
                  unsigned            interval,
                  void               *userarg,
                  thrdtmr_on_start_t  on_start,
                  thrdtmr_on_timer_t  on_timer,
                  thrdtmr_on_stop_t   on_stop)
{
    /*
     * @memberof thrdtmr_t
     * @brief Constructor.
     *
     * @param self      Object instance.
     * @param interval  Timer interval in milliseconds.
     * @param userarg   A user defined parameter to passed to all callbacks,
     *                  and can be NULL if it is no need to use.
     * @param on_start  A callback function that will be called when timer thread starting up,
     *                  and can be NULL if a callback is not needed.
     * @param on_timer  A callback function that will be called on each timer event,
     *                  and can be NULL if a callback is not needed.
     * @param on_stop   A callback function that will be called when timer thread going terminating,
     *                  and can be NULL if a callback is not needed.
     */
    self->thread.available = false;

    sync_init(&self->up.sync);
    atomic_init(&self->down.go_terminate, false);

    self->interval = interval;

    self->events.userarg  = userarg;
    self->events.on_start = on_start ? on_start : on_start_default;
    self->events.on_timer = on_timer ? on_timer : on_timer_default;
    self->events.on_stop  = on_stop  ? on_stop  : on_stop_default;
}
//------------------------------------------------------------------------------
static
void thrdtmr_deinit(thrdtmr_t *self)
{
    /*
     * @memberof thrdtmr_t
     * @brief Destructor.
     *
     * @param self Object instance.
     */
    thrdtmr_stop_and_wait(self);
}
//------------------------------------------------------------------------------
thrdtmr_t* thrdtmr_create(unsigned            interval,
                          void               *userarg,
                          thrdtmr_on_start_t  on_start,
                          thrdtmr_on_timer_t  on_timer,
                          thrdtmr_on_stop_t   on_stop)
{
    /**
     * @memberof thrdtmr_t
     * @brief Create a timer object.
     *
     * @param interval  Timer interval in milliseconds.
     * @param userarg   A user defined parameter to passed to all callbacks,
     *                  and can be NULL if it is no need to use.
     * @param on_start  A callback function that will be called when timer thread starting up,
     *                  and can be NULL if a callback is not needed.
     * @param on_timer  A callback function that will be called on each timer event,
     *                  and can be NULL if a callback is not needed.
     * @param on_stop   A callback function that will be called when timer thread going terminating,
     *                  and can be NULL if a callback is not needed.
     * @return The new object instance, or NULL if failed!
     */
    thrdtmr_t *inst = malloc(sizeof(thrdtmr_t));

    if( inst )
        thrdtmr_init(inst, interval, userarg, on_start, on_timer, on_stop);

    return inst;
}
//------------------------------------------------------------------------------
void thrdtmr_release(thrdtmr_t *self)
{
    /**
     * @memberof thrdtmr_t
     * @brief Release a timer object.
     *
     * @param self Object instance.
     */
    if( self )
    {
        thrdtmr_deinit(self);
        free(self);
    }
}
//------------------------------------------------------------------------------
unsigned thrdtmr_get_interval(const thrdtmr_t *self)
{
    /**
     * @memberof thrdtmr_t
     * @brief Get timer interval.
     *
     * @param self Object instance.
     * @return The timer interval in milliseconds.
     */
    return self->interval;
}
//------------------------------------------------------------------------------
void thrdtmr_set_interval(thrdtmr_t *self, unsigned value)
{
    /**
     * @memberof thrdtmr_t
     * @brief Set timer interval.
     *
     * @param self  Object instance.
     * @param value The time interval in milliseconds to set.
     */
    self->interval = value;
}
//------------------------------------------------------------------------------
static
int THRDS_CALL thread_process(thrdtmr_t *self)
{
    self->up.res = self->events.on_start(self->events.userarg);
    sync_wait(&self->up.sync);

    if( self->up.res )
        return self->events.on_stop(self->events.userarg);

    unsigned time_prev = systime_get_clock_count();
    while( !atomic_load(&self->down.go_terminate) )
    {
        if( self->interval )
        {
            // Run in interval count mode.
            if( systime_get_clock_count() - time_prev >= self->interval )
            {
                self->events.on_timer(self->events.userarg);
                if( atomic_load(&self->down.go_terminate) ) break;

                time_prev += self->interval;
            }
        }
        else
        {
            // Run in interval ignored mode.
            time_prev = systime_get_clock_count();
            self->events.on_timer(self->events.userarg);
        }

        systime_sleep_awhile();
    }

    return self->events.on_stop(self->events.userarg);
}
//------------------------------------------------------------------------------
int thrdtmr_start(thrdtmr_t *self)
{
    /**
     * @memberof thrdtmr_t
     * @brief Start the timer.
     *
     * @param self Object instance.
     * @retval ZERO if succeed.
     * @retval -1 if thread create failed.
     * @retval OTHERS from the user defined ::on_start function.
     */
    thrdtmr_stop_and_wait(self);

    self->up.res = 0;
    sync_reset(&self->up.sync);
    atomic_store(&self->down.go_terminate, false);

    if( thrd_success != thrd_create(&self->thread.instance,
                                    (thrd_start_t) thread_process,
                                    self) )
    {
        return -1;
    }
    self->thread.available = true;

    sync_wait(&self->up.sync);

    if( self->up.res )
        thrdtmr_stop_and_wait(self);

    return self->up.res;
}
//------------------------------------------------------------------------------
int thrdtmr_stop(thrdtmr_t *self, bool wait_stopped)
{
    /**
     * @memberof thrdtmr_t
     * @private
     * @brief Terminate the timer.
     *
     * @param self         Object instance.
     * @param wait_stopped TRUE to block and wait timer terminated; and
     *                     FALSE to return immediately.
     * @return The user defined value be returned from ::on_stop in block mode;
     *         or ZERO for default.
     */
    if( !self->thread.available ) return 0;

    if( thrd_equal(self->thread.instance, thrd_current()) )
        wait_stopped = false;

    int res = 0;
    if( !atomic_exchange(&self->down.go_terminate, true) )
    {
        if( wait_stopped )
            thrd_join(self->thread.instance, &res);
        else
            thrd_detach(self->thread.instance);

        self->thread.available = false;
    }

    if( wait_stopped )
    {
        while( self->thread.available )
            systime_sleep_awhile();
    }

    return res;
}
//------------------------------------------------------------------------------
bool thrdtmr_is_started(const thrdtmr_t *self)
{
    /**
     * @memberof thrdtmr_t
     * @brief Check if the timer is started.
     *
     * @param self Object instance.
     * @return TRUE if the timer is started; and FALSE if not.
     */
    return self->thread.available;
}
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdbool.h>
#include <time.h>

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "timeinf.h"
#include "systime.h"

static long long time_shift_utc;
static long long time_shift_local;

//------------------------------------------------------------------------------
void systime_sleep(unsigned ms)
{
    /**
     * 讓呼叫這個函式的執行緒休眠一段時間。
     * @param ms 欲休眠的時間(milliseconds)。
     */
#if   defined(__linux__)
    static const unsigned ms_max = ((unsigned)-1) / 1000;
    usleep( 1000 * (( ms > ms_max )?( ms_max ):( ms )) );
#elif defined(_WIN32)
    Sleep(ms);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void systime_sleep_awhile(void)
{
    /**
     * 讓呼叫這個函式的執行緒休眠一段很小的時間，主要的目的是讓執行緒放出 CPU 資源，
     * 對於解決 Busy Wait 的需求很有幫助。
     */
#if   defined(__linux__)
    usleep(1);
#elif defined(_WIN32)
    Sleep(1);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
unsigned systime_get_clock_count(void)
{
    /**
     * 取得系統從開機到呼叫這個函式時所經過的時間(milliseconds)。
     * @note Users will need to link with "rt" library for glibc version before 2.17.
     */
#if   defined(__linux__)

    #if   defined(CLOCK_MONOTONIC_RAW)
    static const clockid_t clockid = CLOCK_MONOTONIC_RAW;
    #elif defined(CLOCK_MONOTONIC)
    static const clockid_t clockid = CLOCK_MONOTONIC;
    #else
    static const clockid_t clockid = CLOCK_REALTIME;
    #endif

    struct timespec tp;
    int             gettime_result;

    gettime_result = clock_gettime(clockid, &tp);
    assert( gettime_result == 0 );

    return tp.tv_sec*1000 + tp.tv_nsec/1000000;

#elif defined(_WIN32)

    #if WINVER > 0x0600
    return GetTickCount64();
    #else
    return GetTickCount();
    #endif

#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
static
timeinf_t timeinf_from_stdtm(struct tm *stdtm)
{
    timeinf_t timeinf;

    timeinf.year   = stdtm->tm_year + 1900;
    timeinf.month  = stdtm->tm_mon  + 1;
    timeinf.day    = stdtm->tm_mday;
    timeinf.hour   = stdtm->tm_hour;
    timeinf.minute = stdtm->tm_min;
    timeinf.second = stdtm->tm_sec;

    return timeinf;
}
//------------------------------------------------------------------------------
static
void calc_time_shift(void)
{
    time_t    time_curr     = time(NULL);
    timeinf_t timeinf_utc   = timeinf_from_stdtm(gmtime   (&time_curr));
    timeinf_t timeinf_local = timeinf_from_stdtm(localtime(&time_curr));
    long long time_utc      = timeinf_to_uxtime(&timeinf_utc);
    long long time_local    = timeinf_to_uxtime(&timeinf_local);

    time_shift_utc   = time_utc   - time_curr;
    time_shift_local = time_local - time_curr;
}
//------------------------------------------------------------------------------
int systime_get_timezone(void)
{
    /**
     * 取得本地的時區。
     * @return 傳回本地時間與 UTC 時間的差距(Hours)。
     */
    static bool initialized = false;
    static int  timezone;

    if( !initialized )
    {
        calc_time_shift();
        timezone = (int)( ( time_shift_local - time_shift_utc )/3600 );

        initialized = true;
    }

    return timezone;
}
//------------------------------------------------------------------------------
long long systime_get_utc(void)
{
    /**
     * 取得當前的 UTC 時間。
     * @return UNIX time stamp (UTC).
     */
    static bool initialized = false;

    if( !initialized )
    {
        calc_time_shift();
        initialized = true;
    }

    return time(NULL) + time_shift_utc;
}
//------------------------------------------------------------------------------
long long systime_get_local(void)
{
    /**
     * 取得當前的本地時間。
     * @return UNIX time stamp (local).
     */
    static bool initialized = false;

    if( !initialized )
    {
        calc_time_shift();
        initialized = true;
    }

    return time(NULL) + time_shift_local;
}
//------------------------------------------------------------------------------

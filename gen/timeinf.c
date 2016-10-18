#include <assert.h>
#include <limits.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "timeinf.h"

static const long long seconds_of_minute = 60;
static const long long seconds_of_hour   = 60*60;
static const long long seconds_of_day    = 60*60*24;

static const timeinf_t timeinf_origin  = { 1970, 1, 1, 0, 0, 0 };
static const timeinf_t timeinf_invalid = {    0, 0, 0, 0, 0, 0 };

//------------------------------------------------------------------------------
//---- Common Tools ------------------------------------------------------------
//------------------------------------------------------------------------------
static
bool is_uxtime_valid(long long uxtime)
{
    static const long long uxtime_min = -2208988800LL;   // 0000/01/01 00:00:00
    static const long long uxtime_max = 253402300799LL;  // 9999/12/31 23:59:59
    return uxtime_min <= uxtime && uxtime <= uxtime_max;
}
//------------------------------------------------------------------------------
static
bool is_timeinf_valid(const timeinf_t *info)
{
    return 0 <= info->year   && info->year    <= 9999 &&
           1 <= info->month  && info->month   <=   12 &&
           1 <= info->day    && info->day     <=   31 &&
           0 <= info->hour   && info->hour    <=   23 &&
           0 <= info->minute && info->minute  <=   59 &&
           0 <= info->second && info->second  <=   59;
}
//------------------------------------------------------------------------------
static
bool is_leap_year(unsigned year)
{
    return !(year%400) || ( !(year%4) && !!(year%100) );
}
//------------------------------------------------------------------------------
static
long long count_days_of_year(unsigned year)
{
    return ( is_leap_year(year) )?( 366 ):( 365 );
}
//------------------------------------------------------------------------------
static
long long count_days_of_month(unsigned month, unsigned year)
{
    switch( month )
    {
    case  1:  return 31;
    case  2:  return ( is_leap_year(year) )?( 29 ):( 28 );
    case  3:  return 31;
    case  4:  return 30;
    case  5:  return 31;
    case  6:  return 30;
    case  7:  return 31;
    case  8:  return 31;
    case  9:  return 30;
    case 10:  return 31;
    case 11:  return 30;
    case 12:  return 31;
    default:  return 0;
    }
}
//------------------------------------------------------------------------------
static
long long count_seconds_of_year(unsigned year)
{
    return seconds_of_day * count_days_of_year(year);
}
//------------------------------------------------------------------------------
static
long long count_seconds_of_month(unsigned month, unsigned year)
{
    return seconds_of_day * count_days_of_month(month, year);
}
//------------------------------------------------------------------------------
//---- UNIX Timestamp to Digits ------------------------------------------------
//------------------------------------------------------------------------------
static
long long analysis_year_from_seconds(long long /*in*/ seconds, unsigned* /*out*/ year)
{
    unsigned  year_curr = timeinf_origin.year;
    long long seconds_of_year;
    long long seconds_passed = 0;

    assert( year );

    if( seconds >= 0 )
    {
        seconds_of_year = count_seconds_of_year(year_curr);
        while( seconds_of_year <= seconds )
        {
            seconds_passed  += seconds_of_year;
            seconds         -= seconds_of_year;
            seconds_of_year  = count_seconds_of_year( ++year_curr );
        }
    }
    else
    {
        while( seconds < 0 )
        {
            seconds_of_year  = count_seconds_of_year( --year_curr );
            seconds_passed  -= seconds_of_year;
            seconds         += seconds_of_year;
        }
    }

    *year = year_curr;
    return seconds_passed;
}
//------------------------------------------------------------------------------
static
long long analysis_month_from_seconds(long long /*in*/  seconds,
                                      unsigned* /*out*/ month,
                                      unsigned  /*in*/  year)
{
    unsigned  month_curr = timeinf_origin.month;
    long long seconds_of_month;
    long long seconds_passed = 0;

    assert( month );

    seconds_of_month = count_seconds_of_month(month_curr, year);
    while( seconds_of_month <= seconds )
    {
        seconds_passed   += seconds_of_month;
        seconds          -= seconds_of_month;
        seconds_of_month  = count_seconds_of_month( ++month_curr, year );
    }

    *month = month_curr;
    return seconds_passed;
}
//------------------------------------------------------------------------------
static
long long analysis_day_from_seconds(long long /*in*/ seconds, unsigned* /*out*/ day)
{
    unsigned  day_passed;
    long long seconds_passed = 0;

    assert( day );

    day_passed      = (unsigned)( seconds / seconds_of_day );
    seconds_passed += day_passed * seconds_of_day;

    *day = day_passed + 1;
    return seconds_passed;
}
//------------------------------------------------------------------------------
static
long long analysis_time_from_seconds(long long /*in*/  seconds,
                                     unsigned* /*out*/ hour,
                                     unsigned* /*out*/ minute,
                                     unsigned* /*out*/ second)
{
    long long seconds_passed = seconds;

    assert( hour && minute && second );

    *hour    = (unsigned)( seconds / seconds_of_hour );
    seconds -= (*hour) * seconds_of_hour;

    *minute  = (unsigned)( seconds / seconds_of_minute );
    seconds -= (*minute) * seconds_of_minute;

    *second  = (unsigned)seconds;

    return seconds_passed;
}
//------------------------------------------------------------------------------
timeinf_t timeinf_from_uxtime(long long uxtime)
{
    /**
     * @brief Translate an UNIX time stamp to the time information.
     *
     * @param uxtime The UNIX time stamp.
     * @return The equivalence time information.
     */
    if( !is_uxtime_valid(uxtime) ) return timeinf_invalid;

    timeinf_t timeinf = timeinf_origin;

    long long seconds = uxtime;
    seconds -= analysis_year_from_seconds (seconds, &timeinf.year);
    seconds -= analysis_month_from_seconds(seconds, &timeinf.month, timeinf.year);
    seconds -= analysis_day_from_seconds  (seconds, &timeinf.day);
    seconds -= analysis_time_from_seconds (seconds, &timeinf.hour,
                                                    &timeinf.minute,
                                                    &timeinf.second);

    return timeinf;
}
//------------------------------------------------------------------------------
//---- Digits to UNIX Time Stamp -----------------------------------------------
//------------------------------------------------------------------------------
static
long long calculate_seconds_to_target_year(unsigned year_target)
{
    unsigned  year;
    long long result = 0;

    if( year_target >= timeinf_origin.year )
    {
        for(year = timeinf_origin.year;
            year < year_target;
            ++year)
        {
            result += count_seconds_of_year(year);
        }
    }
    else
    {
        for(year = year_target;
            year < timeinf_origin.year;
            ++year)
        {
            result -= count_seconds_of_year(year);
        }
    }

    return result;
}
//------------------------------------------------------------------------------
static
long long calculate_seconds_to_target_month(unsigned month_target, unsigned year)
{
    unsigned  month;
    long long result = 0;

    for(month = timeinf_origin.month;
        month < month_target;
        ++month)
    {
        result += count_seconds_of_month(month, year);
    }

    return result;
}
//------------------------------------------------------------------------------
static
long long calculate_seconds_to_target_day(unsigned day_target)
{
    return ( day_target - 1 ) * seconds_of_day;
}
//------------------------------------------------------------------------------
static
long long calculate_seconds_of_time(unsigned hour, unsigned minute, unsigned second)
{
    return hour   * seconds_of_hour   +
           minute * seconds_of_minute +
           second;
}
//------------------------------------------------------------------------------
long long timeinf_to_uxtime(const timeinf_t *timeinf)
{
    /**
     * @brief Translate a time information to UNIX time stamp
     *
     * @param timeinf The time information.
     * @return The equivalence UNIX time stamp.
     */
    assert( timeinf );

    if( !is_timeinf_valid(timeinf) ) return LLONG_MIN;

    long long result = 0;
    result += calculate_seconds_to_target_year (timeinf->year);
    result += calculate_seconds_to_target_month(timeinf->month, timeinf->year);
    result += calculate_seconds_to_target_day  (timeinf->day);
    result += calculate_seconds_of_time        (timeinf->hour,
                                                timeinf->minute,
                                                timeinf->second);

    return result;
}
//------------------------------------------------------------------------------
//---- Comparison --------------------------------------------------------------
//------------------------------------------------------------------------------
bool timeinf_is_equal(const timeinf_t *timeinf1, const timeinf_t *timeinf2)
{
    /**
     * @brief 比較兩個時間資訊結構是否相等
     *
     * @param timeinf1 The time information 1.
     * @param timeinf2 The time information 2.
     * @return TRUE if the two time information are equivalence; and FALSE if not.
     */
    assert( timeinf1 && timeinf2 );

    return timeinf1->year   == timeinf2->year   &&
           timeinf1->month  == timeinf2->month  &&
           timeinf1->day    == timeinf2->day    &&
           timeinf1->hour   == timeinf2->hour   &&
           timeinf1->minute == timeinf2->minute &&
           timeinf1->second == timeinf2->second;
}
//------------------------------------------------------------------------------
//---- Format Output -----------------------------------------------------------
//------------------------------------------------------------------------------
char* timeinf_to_datestr(const timeinf_t *timeinf, char *buf, size_t bufsz, const char *delimiter)
{
    /**
     * @brief Print date information to a string
     *
     * @param timeinf   The time information to be printed.
     * @param buf       A buffer to receive the result data.
     * @param bufsz     Size of the output buffer.
     * @param delimiter The delimiter to be used to separate each item,
     *                  and can be NULL to use default delimiter.
     * @return @a buf will be returned.
     */
    if( !buf || !bufsz ) return buf;

    delimiter = delimiter ? delimiter : "";

    snprintf(buf,
             bufsz,
             "%u%s%02u%s%02u",
             timeinf->year,
             delimiter,
             timeinf->month,
             delimiter,
             timeinf->day);

    return buf;
}
//------------------------------------------------------------------------------
char* timeinf_to_timestr(const timeinf_t *timeinf, char *buf, size_t bufsz, const char *delimiter)
{
    /**
     * @brief Print time (exclude date) information to a string
     *
     * @param timeinf   The time information to be printed.
     * @param buf       A buffer to receive the result data.
     * @param bufsz     Size of the output buffer.
     * @param delimiter The delimiter to be used to separate each item,
     *                  and can be NULL to use default delimiter.
     * @return @a buf will be returned.
     */
    if( !buf || !bufsz ) return buf;

    delimiter = delimiter ? delimiter : "";

    snprintf(buf,
             bufsz,
             "%02u%s%02u%s%02u",
             timeinf->hour,
             delimiter,
             timeinf->minute,
             delimiter,
             timeinf->second);

    return buf;
}
//------------------------------------------------------------------------------
char* timeinf_to_datetimestr(const timeinf_t *timeinf,
                             char            *buf,
                             size_t           bufsz,
                             const char      *datedeli,
                             const char      *timedeli,
                             const char      *middeli)
{
    /**
     * @brief Print time (exclude date) information to a string
     *
     * @param timeinf  The time information to be printed.
     * @param buf      A buffer to receive the result data.
     * @param bufsz    Size of the output buffer.
     * @param datedeli The delimiter to be used to format date string,
     *                  and can be NULL to use default delimiter.
     * @param timedeli The delimiter to be used to format date string,
     *                  and can be NULL to use default delimiter.
     * @param middeli  The delimiter to be used between date and time string,
     *                  and can be NULL to use default delimiter.
     * @return @a buf will be returned.
     */
    if( !buf || !bufsz ) return buf;

    datedeli = datedeli ? datedeli : "";
    timedeli = timedeli ? timedeli : "";
    middeli  = middeli  ? middeli  : "";

    snprintf(buf,
             bufsz,
             "%u%s%02u%s%02u" "%s" "%02u%s%02u%s%02u",
             timeinf->year,
             datedeli,
             timeinf->month,
             datedeli,
             timeinf->day,
             middeli,
             timeinf->hour,
             timedeli,
             timeinf->minute,
             timedeli,
             timeinf->second);

    return buf;
}
//------------------------------------------------------------------------------
//---- Others ------------------------------------------------------------------
//------------------------------------------------------------------------------
unsigned timeinf_get_day_of_year(const timeinf_t *timeinf)
{
    /**
     * @brief 計算指定日期是該年的第幾天
     *
     * @param timeinf The time information.
     * @return Days of the year.
     */
    assert( timeinf );

    unsigned days  = 0;
    unsigned month = timeinf->month;
    while( month-- ) days += count_days_of_month(month, timeinf->year);
    days += timeinf->day;

    return days;
}
//------------------------------------------------------------------------------

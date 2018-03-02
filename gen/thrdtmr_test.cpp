#include <assert.h>
#include <threads.h>
#include <iostream>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "systime.h"
#include "thrdtmr.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

using namespace std;

class CountTimer : public ThreadTimer
{
private:
    int count;

public:
    CountTimer(unsigned Interval) : ThreadTimer(Interval), count(-1) {}

public:
    virtual int  OnStart() { count=0; return 0; }
    virtual void OnTimer() { cout << "Thread count : " << count++ << endl; }
    virtual int  OnStop() { return count; }

};

void CountTest()
{
    static const int test_interval  = 1000; // Milliseconds.
    static const int test_count     = 4;
    static const int test_period    = test_count * test_interval;
    static const int test_times     = 3;
    static const int test_margin    = test_interval / 4;

    CountTimer timer(test_interval);

    for(int n=0; n<test_times; ++n)
    {
        cout << "Test iteration : " << n << "/" << test_times-1 << endl;

        unsigned timestart = systime_get_clock_count();
        timer.Start();

        systime_sleep( test_period + test_margin );

        int      result_count = timer.StopAndWait();
        unsigned timeend      = systime_get_clock_count();
        unsigned timepassed   = timeend - timestart;

        cout << "Terminate count : " << result_count << ", time=" << timepassed << endl;
        assert( test_count == result_count );
        assert( test_period <= timepassed && timepassed <= test_period + 2*test_margin );
        cout << endl;
    }
}

class RaceTimer : public ThreadTimer
{
private:
    bool terminate;

public:
    RaceTimer() : ThreadTimer(0), terminate(false) {}

public:
    virtual int OnStart() { cout << "Race timer: start." << endl; return 0; }
    virtual int OnStop() { cout << "Race timer: stop." << endl; return 0; }

    virtual void OnTimer()
    {
        for(int i=0; i<40; ++i)
        {
            if( terminate )
                StopNotWait();
            else
                systime_sleep_awhile();
        }
    }

public:
    int Terminate()
    {
        terminate = true;
        return StopAndWait();
    }
};

void ParallelRun(RaceTimer *timers, int count, int(*action)(RaceTimer*))
{
    struct RunData
    {
        RaceTimer *timer;
        int(*action)(RaceTimer*);
    };

    RunData data[count];
    for(int i=0; i<count; ++i)
    {
        data[i].timer = &timers[i];
        data[i].action = action;
    }

    int(*thread_function)(RunData *data) =
        [](RunData *data) -> int { return data->action(data->timer); };

    thrd_t threads[count];
    for(int i=0; i<count; ++i)
    {
        assert( thrd_success == thrd_create(&threads[i], (int(*)(void*)) thread_function, &data[i]) );
    }

    for(int i=0; i<count; ++i)
    {
        int res;
        assert( thrd_success == thrd_join(threads[i], &res) );
        assert( res == 0 );
    }
}

void ThreadRaceTest()
{
    static const int timercount = 8;
    RaceTimer timers[timercount];

    cout << "Start timers:" << endl;
    ParallelRun(timers, timercount, [](RaceTimer *timer){ return timer->Start(); });
    cout << endl;

    systime_sleep(1000);

    cout << "Stop timers:" << endl;
    ParallelRun(timers, timercount, [](RaceTimer *timer){ return timer->Terminate(); });
    ParallelRun(timers, timercount, [](RaceTimer *timer){ return timer->Terminate(); });
    cout << endl;
}

int main(int argc, char *argv[])
{
    CountTest();
    ThreadRaceTest();

    return 0;
}

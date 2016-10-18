#include <assert.h>
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

#define TEST_INTERVAL       1000  // milliseconds
#define TEST_COUNT          4
#define TEST_TIME           ( TEST_COUNT * TEST_INTERVAL )

#define TEST_ITER           3
#define TEST_TIME_MARGIN    ( TEST_INTERVAL / 4 )

class TestTimer : public TThreadTimer
{
public:
    int Count;

public:
    TestTimer(unsigned Interval) : TThreadTimer(Interval), Count(-1) {}

public:
    virtual int  OnStartup    () { Count=0; return 0; }
    virtual void OnTimer      () { cout << "Thread count : " << Count++ << endl; }
    virtual int  OnTerminating() { return Count; }

};

int main(int argc, char *argv[])
{
    TestTimer Timer(TEST_INTERVAL);

    for(int n=0; n<TEST_ITER; ++n)
    {
        cout << "Test iteration : " << n << "/" << TEST_ITER-1 << endl;

        unsigned TimeStart = systime_get_clock_count();
        Timer.Start();

        systime_sleep( TEST_TIME + TEST_TIME_MARGIN );

        int      ThreadResult = Timer.TerminateAndWaitTerminated();
        unsigned TimeEnd      = systime_get_clock_count();
        unsigned TimePassed   = TimeEnd - TimeStart;

        cout << "Terminate count : " << ThreadResult << ", time=" << TimePassed << endl;
        assert( TEST_COUNT == ThreadResult );
        assert( TEST_TIME <= TimePassed && TimePassed <= TEST_TIME + 2*TEST_TIME_MARGIN );
        cout << endl;
    }

    return 0;
}

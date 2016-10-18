#include <assert.h>
#include <iostream>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "timeinf.h"
#include "systime.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

using namespace std;

int main()
{
    timeinf_t timeinf;

    cout << "Clock count : " << systime_get_clock_count() << endl;

    cout << "Local time zone : " << systime_get_timezone() << endl;

    timeinf = timeinf_from_uxtime(systime_get_utc());
    cout << "UTC   time : " << timeinf.year   << "/"
                            << timeinf.month  << "/"
                            << timeinf.day    << " "
                            << timeinf.hour   << ":"
                            << timeinf.minute << ":"
                            << timeinf.second << endl;

    timeinf = timeinf_from_uxtime(systime_get_local());
    cout << "Local time : " << timeinf.year   << "/"
                            << timeinf.month  << "/"
                            << timeinf.day    << " "
                            << timeinf.hour   << ":"
                            << timeinf.minute << ":"
                            << timeinf.second << endl;

    return 0;
}

#include <stdio.h>
#include "timectr.h"

int main(int argc, char *argv[])
{
    TimeCounter timer;

    timer.Reset(5*1000);
    for(int i=0; i<8; ++i, systime_sleep(1*1000))
    {
        printf("timeout : %u, passed : %u, remain : %u, expired : %d\n",
               timer.GetTimeout(),
               timer.GetPassed(),
               timer.GetRemain(),
               timer.IsExpired());
    }

    return 0;
}

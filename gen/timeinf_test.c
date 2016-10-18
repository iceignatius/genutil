#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "timeinf.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

typedef struct testsample_t
{
    long long uxtime;
    timeinf_t timeinf;
    unsigned  day_of_year;
} testsample_t;

static const testsample_t testsample[] =
{
    { 2071925370 , { 2035, 8,28,  14,49,30 }, 240 },
    { 1167207562 , { 2006,12,27,   8,19,22 }, 361 },
    { 2126913970 , { 2037, 5,26,   1,26,10 }, 146 },
    { 1184227899 , { 2007, 7,12,   8,11,39 }, 193 },
    { 1786311937 , { 2026, 8, 9,  21,45,37 }, 221 },
    { 723770858  , { 1992,12, 7,  23,27,38 }, 342 },
    { 839113802  , { 1996, 8, 3,  23,10, 2 }, 216 },
    { 350208087  , { 1981, 2, 5,   8, 1,27 },  36 },
    { 334870084  , { 1980, 8,11,  19,28, 4 }, 224 },
    { 24538334   , { 1970,10,12,   0,12,14 }, 285 },
    { 599240541  , { 1988,12,27,  15,42,21 }, 362 },
    { 267174176  , { 1978, 6,20,   7, 2,56 }, 171 },
    { 2110662428 , { 2036,11,18,  23, 7, 8 }, 323 },
    { 205235600  , { 1976, 7, 3,   9,53,20 }, 185 },
    { 1870435127 , { 2029, 4, 9,  13,18,47 },  99 },
    { 51402844   , { 1971, 8,18,  22,34, 4 }, 230 },
    { 1629773244 , { 2021, 8,24,   2,47,24 }, 236 },
    { 980933942  , { 2001, 1,31,   9,39, 2 },  31 },
    { 546977521  , { 1987, 5, 2,  18,12, 1 }, 122 },
    { 1591755047 , { 2020, 6,10,   2,10,47 }, 162 },
    { 2220466    , { 1970, 1,26,  16,47,46 },  26 },
    { 1867992827 , { 2029, 3,12,   6,53,47 },  71 },
    { 2007011383 , { 2033, 8, 7,   7, 9,43 }, 219 },
    { 860615025  , { 1997, 4, 9,  19,43,45 },  99 },
    { 348520007  , { 1981, 1,16,  19, 6,47 },  16 },
    { 61786883   , { 1971,12,17,   3, 1,23 }, 351 },
    { 850214845  , { 1996,12,10,  10,47,25 }, 345 },
    { 82786230   , { 1972, 8,16,   4,10,30 }, 229 },
    { 849481326  , { 1996,12, 1,  23, 2, 6 }, 336 },
    { 1122000194 , { 2005, 7,22,   2,43,14 }, 203 },
    { 945876135  , { 1999,12,22,  15,22,15 }, 356 },
    { 773923048  , { 1994, 7,11,  10,37,28 }, 192 },
    { 141724108  , { 1974, 6,29,   7,48,28 }, 180 },
    { 925306457  , { 1999, 4,28,  13,34,17 }, 118 },
    { 1958150947 , { 2032, 1,19,  18,49, 7 },  19 },
    { 1928036046 , { 2031, 2, 5,   5,34, 6 },  36 },
    { 1649077316 , { 2022, 4, 4,  13, 1,56 },  94 },
    { 649781102  , { 1990, 8, 4,  14,45, 2 }, 216 },
    { 130760485  , { 1974, 2,22,  10,21,25 },  53 },
    { 1983947400 , { 2032,11,13,   8,30, 0 }, 318 },
    { -945876135 , { 1940, 1,11,   8,37,45 },  11 },
    { -773923048 , { 1945, 6,23,  13,22,32 }, 174 },
    { -141724108 , { 1965, 7, 5,  16,11,32 }, 186 },
    { -925306457 , { 1940, 9, 5,  10,25,43 }, 249 },
    { -1958150947, { 1907,12,14,   5,10,53 }, 348 },
    { -1928036046, { 1908,11,26,  18,25,54 }, 331 },
    { -1649077316, { 1917, 9,29,  10,58, 4 }, 272 },
    { -649781102 , { 1949, 5,30,   9,14,58 }, 150 },
    { -130760485 , { 1965,11, 9,  13,38,35 }, 313 },
    { -1983947400, { 1907, 2,18,  15,30, 0 },  49 }
};

static const size_t testsample_count = sizeof(testsample)/sizeof(testsample[0]);

int main(int argc, char *argv[])
{
    int i;

    for(i=0 ;i<testsample_count; ++i)
    {
        timeinf_t timeinf = timeinf_from_uxtime(testsample[i].uxtime);
        assert( timeinf_is_equal(&timeinf, &testsample[i].timeinf) );
    }

    for(i=0 ;i<testsample_count; ++i)
    {
        long long uxtime = timeinf_to_uxtime(&testsample[i].timeinf);
        assert( uxtime == testsample[i].uxtime );
    }

    {
        static const timeinf_t timeinf       = { 1995, 3, 27, 13, 16, 9 };
        static const char      datestr[]     = "1995/03/27";
        static const char      timestr[]     = "13:16:09";
        static const char      datetimestr[] = "1995/03/27 13:16:09";
        char buf[128];

        timeinf_to_datestr(&timeinf, buf, sizeof(buf), "/");
        assert( 0 == strcmp(buf, datestr) );

        timeinf_to_timestr(&timeinf, buf, sizeof(buf), ":");
        assert( 0 == strcmp(buf, timestr) );

        timeinf_to_datetimestr(&timeinf, buf, sizeof(buf), "/", ":", " ");
        assert( 0 == strcmp(buf, datetimestr) );
    }

    for(i=0 ;i<testsample_count; ++i)
    {
        unsigned day = timeinf_get_day_of_year(&testsample[i].timeinf);
        assert( day == testsample[i].day_of_year );
    }

    return 0;
}

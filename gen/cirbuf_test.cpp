#include <assert.h>
#include <string.h>
#include "cirbuf.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

int main(void)
{
    // Prepare circular buffer object.

    TCirBuf cirbuf;

    size_t totalsize = cirbuf.Allocate(30);
    assert( totalsize >= 30 );
    assert(  0 == cirbuf.GetDataSize() );
    assert( totalsize == cirbuf.GetFreeSize() );

    static const int test_rounds = 10*1024;
    for(int i=0; i<test_rounds; ++i)
    {
        static const char data[24] =
        {
            'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l',
            'm', 'n', 'o', 'p', 'q', 'r',
            's', 't', 'u', 'v', 'w', 'x'
        };

        assert( 6 == cirbuf.Write( data + 0*6, 6) );
        assert( 6 == cirbuf.Write( data + 1*6, 6) );
        assert( 6 == cirbuf.Write( data + 2*6, 6) );
        assert( 6 == cirbuf.Write( data + 3*6, 6) );
        assert( 24 == cirbuf.GetDataSize() );
        assert( totalsize - 24 == cirbuf.GetFreeSize() );

        char buf[24] = {0};

        assert( 24 == cirbuf.Read(buf, 24) );
        assert( 0 == cirbuf.GetDataSize() );
        assert( totalsize == cirbuf.GetFreeSize() );

        assert( 0 == memcmp(buf, data, sizeof(data)) );
    }

    return 0;
}

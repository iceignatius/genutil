/*
 * mempool 測試程式
 */
#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "mempool.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

typedef struct testunit_t
{
    char       *buffer;  // Buffer allocated from the pool
    const char *sample;  // The sample data (a null-terminated string) to be set to the buffer.
} testunit_t;

static testunit_t testvars[] =
{
    { NULL, "AAAAAAA" },
    { NULL, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBB" },
    { NULL, "CCCCCCCCCC" },
    { NULL, "DDDDDDD" },
    { NULL, "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" },

    { NULL, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" },
    { NULL, "GGGGGGGGGGGGGGGGGGGG" },
    { NULL, "HHHHHHHHHHHHHHHHHHHHHHHHHHHHH" },
    { NULL, "II" },
    { NULL, "JJJJJJJJJJJJJJJJJJJJJ" },

    { NULL, "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK" },
    { NULL, "LLLLLLLLLLLLLLLLLLLLL" },
    { NULL, "MMMMMMM" },
    { NULL, "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN" },
    { NULL, "OOOOOOOOOOOOOOOOOOOOOOOOOO" },

    { NULL, "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP" },
    { NULL, "QQQ" },
    { NULL, "RRRRRRRRRRR" },
    { NULL, "SSSSSSSSSS" },
    { NULL, "TTTTT" },

    { NULL, "UUUUUUUUUUUUUUUUUUUUUUUUU" },
    { NULL, "VVVVVVVVVVVVVVVVV" },
    { NULL, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW" },
    { NULL, "XX" },
    { NULL, "YYYYYYYY" },

    { NULL, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ" },
    { NULL, "00000000000000" },
    { NULL, "11111111111111111111111111111111111111111111111111" },
    { NULL, "22222222222222222" },
    { NULL, "333" }
};

static const size_t testcnt = sizeof(testvars)/sizeof(testvars[0]);

bool index_magic(size_t index)
{
    /*
     * 利用比較混亂的計算排列來決定某個引數的位置是否需要保留
     */
    return ( index / 3 % 3 ) == ( index % 3 );
}

int main(int argc, char *argv[])
{
    byte_t     poolbuf[4*1024];
    mempool_t *pool;
    size_t     freetotal;

    // Initialize
    memset(poolbuf, 0xEF, sizeof(poolbuf));
    assert( !mempool_init(poolbuf, 10) );
    pool = mempool_init(poolbuf, sizeof(poolbuf));
    assert( pool );
    assert( sizeof(poolbuf) == mempool_get_pool_size(pool) );

    // Test and make backup of the original free size.
    freetotal = mempool_get_free_size(pool);
    assert( 0 < freetotal && freetotal < sizeof(poolbuf) );

    // Single buffer allocation test
    {
        char *buf;

        buf = (char*) mempool_allocate(pool, 0);
        assert( buf );
        assert( 0 == mempool_get_buffer_size(pool, buf) );
        assert( freetotal > mempool_get_free_size(pool) );

        mempool_deallocate(pool, buf);
        assert( freetotal == mempool_get_free_size(pool) );
    }
    {
        char     *buf;
        uint64_t  off;

        // Allocate test
        buf = (char*) mempool_allocate(pool, 20);
        assert( buf );
        assert( 20 == mempool_get_buffer_size(pool, buf) );
        assert( freetotal > mempool_get_free_size(pool) );

        // Offset calculation test
        assert( 0    == mempool_offset_from_addr(pool, NULL) );
        assert( NULL == mempool_offset_to_addr  (pool, 0   ) );
        off = mempool_offset_from_addr(pool, buf);
        assert( off );
        assert( buf == mempool_offset_to_addr(pool, off) );

        // Deallocate test
        mempool_deallocate(pool, buf);
        assert( freetotal == mempool_get_free_size(pool) );
    }

    // Multi-buffer allocation test
    {
        size_t i;

        // Allocate buffers
        for(i=0; i<testcnt; ++i)
        {
            size_t size = strlen(testvars[i].sample) + 1;

            testvars[i].buffer = (char*) mempool_allocate(pool, size);
            assert( testvars[i].buffer );
            assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
        }

        // Check pool size
        assert( freetotal > mempool_get_free_size(pool) );

        // Fill data
        for(i=0; i<testcnt; ++i)
        {
            strcpy(testvars[i].buffer, testvars[i].sample);
        }

        // Check data
        for(i=0; i<testcnt; ++i)
        {
            assert( 0 == strcmp(testvars[i].buffer, testvars[i].sample) );
        }

        // Deallocate all buffers
        for(i=0; i<testcnt; ++i)
        {
            mempool_deallocate(pool, testvars[i].buffer);
            testvars[i].buffer = NULL;
        }
        assert( freetotal == mempool_get_free_size(pool) );
    }

    // Multi-buffer repeating allocate and deallocate
    {
        size_t i;

        // Allocate buffers
        for(i=0; i<testcnt; ++i)
        {
            size_t size = strlen(testvars[i].sample) + 1;

            testvars[i].buffer = (char*) mempool_allocate(pool, size);
            assert( testvars[i].buffer );
            assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
        }

        // Deallocate some buffers
        for(i=0; i<testcnt; ++i)
        {
            if( !index_magic(i) )
            {
                mempool_deallocate(pool, testvars[i].buffer);
                testvars[i].buffer = NULL;
            }
        }

        // Get back the buffers we just deallocated (but do it from the end to the head)
        for(i=testcnt-1; i<testcnt; --i)
        {
            if( !testvars[i].buffer )
            {
                size_t size = strlen(testvars[i].sample) + 1;

                testvars[i].buffer = (char*) mempool_allocate(pool, size);
                assert( testvars[i].buffer );
                assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
            }
        }

        // Check pool size
        assert( freetotal > mempool_get_free_size(pool) );

        // Fill data
        for(i=0; i<testcnt; ++i)
        {
            strcpy(testvars[i].buffer, testvars[i].sample);
        }

        // Check data
        for(i=0; i<testcnt; ++i)
        {
            assert( 0 == strcmp(testvars[i].buffer, testvars[i].sample) );
        }

        // Deallocate all buffers
        for(i=0; i<testcnt; ++i)
        {
            mempool_deallocate(pool, testvars[i].buffer);
            testvars[i].buffer = NULL;
        }
        assert( freetotal == mempool_get_free_size(pool) );
    }

    // Buffer reallocate test
    {
        size_t i;

        // Allocate buffers
        for(i=0; i<testcnt; ++i)
        {
            size_t size = strlen(testvars[i].sample) + 1;

            testvars[i].buffer = (char*) mempool_allocate(pool, size);
            assert( testvars[i].buffer );
            assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
        }

        // Check pool size
        assert( freetotal > mempool_get_free_size(pool) );

        // Fill data
        for(i=0; i<testcnt; ++i)
        {
            strcpy(testvars[i].buffer, testvars[i].sample);
        }

        // Reallocate all buffers double size (but do it from the end to the head)
        for(i=testcnt-1; i<testcnt; --i)
        {
            size_t size = 2 * mempool_get_buffer_size(pool, testvars[i].buffer);
            testvars[i].buffer = (char*) mempool_reallocate(pool, testvars[i].buffer, size);
            assert( testvars[i].buffer );
            assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
        }

        // Check data
        for(i=0; i<testcnt; ++i)
        {
            assert( 0 == strcmp(testvars[i].buffer, testvars[i].sample) );
        }

        // Deallocate all buffers
        for(i=0; i<testcnt; ++i)
        {
            mempool_deallocate(pool, testvars[i].buffer);
            testvars[i].buffer = NULL;
        }
        assert( freetotal == mempool_get_free_size(pool) );
    }

    // Buffer seek test
    {
        size_t  i;
        char   *buf;

        // Allocate buffers
        for(i=0; i<testcnt; ++i)
        {
            size_t size = strlen(testvars[i].sample) + 1;

            testvars[i].buffer = (char*) mempool_allocate(pool, size);
            assert( testvars[i].buffer );
            assert( size == mempool_get_buffer_size(pool, testvars[i].buffer) );
        }

        // Deallocate buffers with odd index
        for(i=0; i<testcnt; ++i)
        {
            if( i % 2 )
            {
                mempool_deallocate(pool, testvars[i].buffer);
                testvars[i].buffer = NULL;
            }
        }

        // Seek all buffers allocated
        buf = (char*) mempool_get_inuse_first(pool);
        for(i=0; i<testcnt; i+=2)
        {
            assert( testvars[i].buffer == buf );
            buf = (char*) mempool_get_inuse_next(pool, buf);
        }
        assert( NULL == buf );

        // Deallocate all buffers
        for(i=0; i<testcnt; ++i)
        {
            mempool_deallocate(pool, testvars[i].buffer);
            testvars[i].buffer = NULL;
        }
        assert( freetotal == mempool_get_free_size(pool) );
    }

    return 0;
}

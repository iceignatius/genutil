/*
 * shrdmem 測試程式
 */
#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "shrdmem.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

int main()
{
    static const size_t testsize            = 7;
    static const byte_t testdata1[testsize] = {1,3,5,7,2,4,6};
    static const byte_t testdata2[testsize] = {7,6,5,4,3,2,1};
    static const byte_t testzeros[testsize] = {0,0,0,0,0,0,0};
    static const char   testname []         = "test_shared_memory_name";
    shrdmem_t *shrd_a       = NULL;
    shrdmem_t *shrd_a_clone = NULL;
    shrdmem_t *shrd_b       = NULL;

    // Clear the global shared memory
    assert( shrd_a = shrdmem_create_open(testname, testsize, false) );
    shrdmem_release_s(&shrd_a);
    assert( !shrd_a );

    // Check that the test shared memory does not exist yet
    assert( !shrdmem_query_existed(testname) );
    assert( !shrdmem_create_open_existed(testname) );

    // Create a new shared memory
    assert( shrd_a = shrdmem_create_open(testname, testsize, true) );
    assert( shrdmem_query_existed(testname) );
    assert( shrdmem_is_opened(shrd_a) );
    assert( shrd_a->buf );
    assert( shrd_a->size >= testsize );
    // Check with the fail_if_existed flag
    assert( !shrdmem_create_open(testname, testsize, true) );
    assert( shrd_a_clone = shrdmem_create_open(testname, testsize, false) );
    assert( shrdmem_is_opened(shrd_a_clone) );
    assert( shrdmem_query_existed(testname) );

    // Open the same shared memory with another object
    assert( shrd_b = shrdmem_create_open_existed(testname) );
    assert( shrdmem_is_opened(shrd_b) );
    assert( shrd_b->buf );
    assert( shrd_b->size == shrd_a->size );

    // Data translate test
    memcpy(shrd_a->buf, testdata1, testsize);
    assert( 0 == memcmp(shrd_b->buf, testdata1, testsize) );
    memcpy(shrd_b->buf, testdata2, testsize);
    assert( 0 == memcmp(shrd_a->buf, testdata2, testsize) );
    // Zero memory test
    shrdmem_set_zeros(shrd_a);
    assert( 0 == memcmp(shrd_a->buf, testzeros, testsize) );
    assert( 0 == memcmp(shrd_b->buf, testzeros, testsize) );

    // Close shared memory
    shrdmem_close(shrd_a);
    shrdmem_close(shrd_a_clone);
    shrdmem_close(shrd_b);
    assert( !shrdmem_is_opened(shrd_a) );
    assert( !shrdmem_is_opened(shrd_a_clone) );
    assert( !shrdmem_is_opened(shrd_b) );
    // Check that the shared memory does not exist anymore
    assert( !shrdmem_query_existed(testname) );

    // Release objects
    shrdmem_release(shrd_a);
    shrdmem_release(shrd_a_clone);
    shrdmem_release(shrd_b);

    return 0;
}

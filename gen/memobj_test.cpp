/*
 * memobj 測試程式
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "memobj.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

void test_global_tools(void)
{
    // Data search
    {
        const byte_t sample[] =
        {
            0x01, 0x05, 0x05, 0x07, 0x02, 0x04, 0x06, 0x08,
            0x01, 0x03, 0x05, 0x07, 0x02, 0x04, 0x06, 0x08,
            0x05, 0x07
        };
        const byte_t pattern[] = { 0x05, 0x07, 0x02 };

        assert(    2 == (byte_t*) memfind (sample, sizeof(sample), pattern, sizeof(pattern)) - sample );
        assert(   10 == (byte_t*) memrfind(sample, sizeof(sample), pattern, sizeof(pattern)) - sample );
        assert( NULL == (byte_t*) memfind (sample, sizeof(sample), "\x03\x08\x09\x02", 4) );
        assert( NULL == (byte_t*) memrfind(sample, sizeof(sample), "\x03\x08\x09\x02", 4) );
    }

    // Data search count and replace;
    {
        const byte_t src[] =
        {
            0x0D, 0x0A,
            's', 't', 'r', 'i', 'n', 'g',
            0x0D, 0x0A,
            'f', 'i', 'l', 'l', 'e', 'd',
            0x0D, 0x0A,
            0x0D, 0x0A,
            'b', 'e', 't', 'w', 'e', 'e', 'n',
        };

        const byte_t pattern[] = { 0x0D, 0x0A };
        const byte_t target [] = { 0x1C, 0x1D, 0x1E };

        const byte_t finaldata[] =
        {
            0x1C, 0x1D, 0x1E,
            's', 't', 'r', 'i', 'n', 'g',
            0x1C, 0x1D, 0x1E,
            'f', 'i', 'l', 'l', 'e', 'd',
            0x1C, 0x1D, 0x1E,
            0x1C, 0x1D, 0x1E,
            'b', 'e', 't', 'w', 'e', 'e', 'n',
        };

        byte_t buf[64] = {0};

        assert( 4 == memfindcount(src, sizeof(src), pattern, sizeof(pattern)) );

        assert( sizeof(finaldata) == memfindreplace(NULL,
                                                    0,
                                                    src,
                                                    sizeof(src),
                                                    pattern,
                                                    sizeof(pattern),
                                                    target,
                                                    sizeof(target)) );
        assert( sizeof(finaldata) == memfindreplace(buf,
                                                    sizeof(buf),
                                                    src,
                                                    sizeof(src),
                                                    pattern,
                                                    sizeof(pattern),
                                                    target,
                                                    sizeof(target)) );
        assert( 0 == memcmp(buf, finaldata, sizeof(finaldata)) );
    }

    // memobj_calc_recommended_size
    {
        size_t size1 = -1;
        size_t size2 = memobj_calc_recommended_size(size1);
        assert( size2 >= size1 );
        for(size1=0; size1<0xFFFFFF; ++size1)
        {
            size2 = memobj_calc_recommended_size(size1);
            assert( size2 >= size1 );
        }
    }
}

void test_memory_object(void)
{
    // Object create and release
    {
        static const size_t size = 0;
        mem_t *mem = NULL;

        assert( mem = mem_create(size) );
        assert( mem->size == size );
        assert( mem->size_total >= size );
        mem_release_s(NULL);
        mem_release_s(&mem);
        mem_release_s(&mem);
        assert( !mem );
    }

    // Object create and import and move
    {
        static const byte_t testdata[] = {1,3,5,7,9};
        mem_t *memsrc  = NULL;
        mem_t *memdest = NULL;

        // Create and import
        assert( memsrc = mem_create_import(testdata, sizeof(testdata)) );
        assert( memsrc->buf );
        assert( memsrc->size == sizeof(testdata) );
        assert( 0 == memcmp(memsrc->buf, testdata, sizeof(testdata)) );
        // Create and move
        assert( memdest = mem_create_move(memsrc) );
        assert( memdest->buf );
        assert( memdest->size == sizeof(testdata) );
        assert( 0 == memcmp(memdest->buf, testdata, sizeof(testdata)) );
        assert( memsrc );
        assert( memsrc->buf );
        assert( memsrc->size == 0 );

        mem_release(memsrc);
        mem_release(memdest);
    }

    // Object create and clone
    {
        static const byte_t testdata[] = {1,3,5,7,9};
        mem_t *memsrc  = NULL;
        mem_t *memdest = NULL;

        assert( memsrc = mem_create(sizeof(testdata)) );
        memcpy(memsrc->buf, testdata, sizeof(testdata));

        assert( memdest = mem_create_clone(memsrc) );
        assert( memdest      != memsrc );
        assert( memdest->buf != memsrc->buf );
        assert( 0 == memcmp(memdest->buf, memsrc->buf, memsrc->size) );

        mem_release(memsrc);
        mem_release(memdest);
    }

    // Set zeros
    {
        static const byte_t zeros[] = "\x0\x0\x0\x0\x0\x0\x0\x0";
        static const size_t size    = sizeof(zeros);
        mem_t *mem = NULL;

        assert( mem = mem_create(size) );

        memset(mem->buf, -1, size);
        mem_set_zeros(mem);
        assert( 0 == memcmp(mem->buf, zeros, size) );

        mem_release(mem);
    }

    // Resize
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        static const size_t testsize   = sizeof(testdata);
        mem_t *mem = NULL;
        size_t   newsize;

        assert( mem = mem_create(testsize) );
        memcpy(mem->buf, testdata, testsize);

        newsize = testsize + testsize/2;
        assert( mem_resize(mem, newsize) );
        assert( mem->size == newsize );
        assert( 0 == memcmp(mem->buf, testdata, std::min(newsize,testsize)) );

        newsize = testsize/2;
        assert( mem_resize(mem, newsize) );
        assert( mem->size == newsize );
        assert( 0 == memcmp(mem->buf, testdata, std::min(newsize,testsize)) );

        mem_release(mem);
    }

    // Import
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        mem_t *mem = NULL;

        assert( mem = mem_create(128) );

        assert( mem_import(mem, testdata, sizeof(testdata)) );
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );

        mem_release(mem);
    }

    // Append
    {
        static const byte_t data_1  [] = "\x01\x02\x03\x04";
        static const byte_t data_2  [] = "\x05\x06\x07\x08";
        static const byte_t data_all[] = "\x01\x02\x03\x04\x00\x05\x06\x07\x08";
        mem_t *mem = NULL;

        assert( mem = mem_create(0) );

        assert( mem_import(mem, data_1, sizeof(data_1)) );
        assert( mem_append(mem, data_2, sizeof(data_2)) );
        assert( mem->size == sizeof(data_all) );
        assert( 0 == memcmp(mem->buf, data_all, mem->size) );

        mem_release(mem);
    }

    // Move
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        mem_t *memsrc  = NULL;
        mem_t *memdest = NULL;

        assert( memsrc  = mem_create_import(testdata, sizeof(testdata)) );
        assert( memdest = mem_create(0)                                 );

        mem_move_from(memdest, memsrc);
        assert( memdest->buf );
        assert( memdest->size == sizeof(testdata) );
        assert( 0 == memcmp(memdest->buf, testdata, sizeof(testdata)) );
        assert( memsrc->buf );
        assert( memsrc->size == 0 );

        mem_release(memsrc);
        mem_release(memdest);
    }

    // Pop front
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        static const size_t popsz      = 4;
        mem_t *mem = NULL;

        assert( mem = mem_create(0) );

        assert( mem_import(mem, testdata, sizeof(testdata)) );
        mem_pop_front(mem, popsz);
        assert( mem->size = sizeof(testdata) - popsz );
        assert( 0 == memcmp(mem->buf, testdata+popsz, sizeof(testdata)-popsz) );

        mem_pop_front(mem, sizeof(testdata));
        assert( mem->size == 0 );

        mem_release_s(&mem);
    }

    // Binary file read and write
    {
        static const char   filename[] = "memobj-binary-temp-file";
        static const byte_t testdata[] = "mem_t load save test string.";
        mem_t *mem = NULL;

        // Save file
        assert( mem = mem_create(0) );
        assert( mem_import(mem, testdata, sizeof(testdata)) );
        assert( mem_save_file(mem, filename) );
        memset(mem->buf, 0, mem->size_total);
        mem_release_s(&mem);

        // Load file
        assert( mem = mem_create(0) );
        assert( mem_load_file(mem, filename) );
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );
        mem_release_s(&mem);

        // Create object and load file
        assert( mem = mem_create_load_file(filename) );
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );
        mem_release_s(&mem);

        // Remove the temporary file
        remove(filename);
    }

    // Text format file output
    {
        static const char   filename[] = "memobj-text-temp-file";
        static const byte_t testdata[] =
        {
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,   0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,   0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,   0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        };

        mem_t *mem = NULL;

        assert( mem = mem_create_import(testdata, sizeof(testdata)) );
        assert( mem_save_text(mem, filename) );
        mem_release_s(&mem);

        printf("Test of the text information output : \n\n");
        assert( mem = mem_create_load_file(filename) );
        assert( mem_append(mem, "\x00", 1) );  // Add null terminator
        printf("%s\n", (char*)mem->buf);
        mem_release_s(&mem);

        remove(filename);
    }
}

void test_fixed_buffer_memory_object(void)
{
    // Initialize
    {
        byte_t   buffer[128];
        memfx_t *mem;

        assert( mem = memfx_init(buffer, sizeof(buffer), sizeof(buffer)) );
        assert( mem->size < sizeof(buffer) );
        memfx_deinit(mem);

        assert( mem = memfx_init(buffer, sizeof(buffer), sizeof(buffer)/2) );
        assert( mem->size == sizeof(buffer)/2 );
        memfx_deinit(mem);

        assert( mem = memfx_init(buffer, sizeof(buffer), 0) );
        assert( mem->size == 0 );
        memfx_deinit(mem);

        assert( mem = memfx_init(buffer, sizeof(memfx_header_t)) );
        assert( mem->size == 0 );
        memfx_deinit(mem);

        assert( !( mem = memfx_init(buffer, sizeof(memfx_header_t)-1) ) );
    }

    // Set zeros
    {
        static const byte_t zeros[] = "\x0\x0\x0\x0\x0\x0\x0\x0";
        static const size_t size    = sizeof(zeros);
        byte_t   buffer[128];
        memfx_t *mem = NULL;

        memset(buffer, -1, sizeof(buffer));
        assert( mem = memfx_init(buffer, sizeof(buffer), sizeof(buffer)) );

        memfx_set_zeros(mem);
        assert( 0 == memcmp(mem->buf, zeros, size) );

        memfx_deinit(mem);
    }

    // Resize
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        static const size_t testsize   = sizeof(testdata);
        byte_t   buffer[128];
        memfx_t *mem = NULL;
        size_t   newsize;

        assert( mem = memfx_init(buffer, sizeof(buffer), testsize) );
        memcpy(mem->buf, testdata, testsize);

        newsize = testsize + testsize/2;
        assert( memfx_resize(mem, newsize) );
        assert( mem->size == newsize );
        assert( 0 == memcmp(mem->buf, testdata, std::min(newsize,testsize)) );

        newsize = testsize/2;
        assert( memfx_resize(mem, newsize) );
        assert( mem->size == newsize );
        assert( 0 == memcmp(mem->buf, testdata, std::min(newsize,testsize)) );

        assert( !memfx_resize(mem, sizeof(buffer)+1) );

        memfx_deinit(mem);
    }

    // Import
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        static const byte_t largedata[128+1] = {0};
        byte_t   buffer[128];
        memfx_t *mem = NULL;

        assert( mem = memfx_init(buffer, sizeof(buffer), sizeof(buffer)) );

        // Normal import
        assert( memfx_import(mem, testdata, sizeof(testdata)) );
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );

        // Import data that large then the object can hold
        assert( !memfx_import(mem, largedata, sizeof(largedata)) );
        // - Check that the object has not be changed
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );

        memfx_deinit(mem);
    }

    // Append
    {
        static const byte_t data_1  [] = "\x01\x02\x03\x04";
        static const byte_t data_2  [] = "\x05\x06\x07\x08";
        static const byte_t data_all[] = "\x01\x02\x03\x04\x00\x05\x06\x07\x08";
        static const byte_t largedata[128-1] = {0};
        byte_t   buffer[128];
        memfx_t *mem = NULL;

        assert( mem = memfx_init(buffer, sizeof(buffer)) );

        // Normal append
        assert( memfx_import(mem, data_1, sizeof(data_1)) );
        assert( memfx_append(mem, data_2, sizeof(data_2)) );
        assert( mem->size == sizeof(data_all) );
        assert( 0 == memcmp(mem->buf, data_all, mem->size) );

        // Append data that large then the object can hold
        assert( !memfx_append(mem, largedata, sizeof(largedata)) );
        // - Check that the object has not be changed
        assert( mem->size == sizeof(data_all) );
        assert( 0 == memcmp(mem->buf, data_all, mem->size) );

        memfx_deinit(mem);
    }

    // Pop front
    {
        static const byte_t testdata[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
        static const size_t popsz      = 4;
        byte_t   buffer[128];
        memfx_t *mem = NULL;

        assert( mem = memfx_init(buffer, sizeof(buffer)) );

        assert( memfx_import(mem, testdata, sizeof(testdata)) );
        memfx_pop_front(mem, popsz);
        assert( mem->size = sizeof(testdata) - popsz );
        assert( 0 == memcmp(mem->buf, testdata+popsz, sizeof(testdata)-popsz) );

        memfx_pop_front(mem, sizeof(testdata));
        assert( mem->size == 0 );

        memfx_deinit(mem);
    }

    // Binary file read and write
    {
        static const char   filename[] = "memobj-binary-temp-file";
        static const byte_t testdata[] = "mem_t load save test string.";
        byte_t   buffer[128];
        memfx_t *mem = NULL;

        // Save file
        assert( mem = memfx_init(buffer, sizeof(buffer)) );
        assert( memfx_import(mem, testdata, sizeof(testdata)) );
        assert( memfx_save_file(mem, filename) );
        memfx_deinit(mem);
        memset(buffer, 0, sizeof(buffer));

        // Load file
        assert( mem = memfx_init(buffer, sizeof(buffer)) );
        assert( memfx_load_file(mem, filename) );
        assert( mem->size == sizeof(testdata) );
        assert( 0 == memcmp(mem->buf, testdata, mem->size) );
        memfx_deinit(mem);
        memset(buffer, 0, sizeof(buffer));

        // Load with small buffer that cannot hold the data
        assert( mem = memfx_init(buffer, sizeof(testdata)-1) );
        assert( !memfx_load_file(mem, filename) );
        memfx_deinit(mem);
        memset(buffer, 0, sizeof(buffer));

        // Remove the temporary file
        remove(filename);
    }

    // Text format file output
    {
        static const char   filename[] = "memobj-text-temp-file";
        static const byte_t testdata[] =
        {
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,   0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
            0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,   0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
            0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,   0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        };

        byte_t   buffer[128*4];
        memfx_t *mem = NULL;

        assert( mem = memfx_init(buffer, sizeof(buffer)) );
        assert( memfx_import(mem, testdata, sizeof(testdata)) );
        assert( memfx_save_text(mem, filename) );
        memfx_deinit(mem);

        printf("Test of the text information output : \n\n");
        assert( mem = memfx_init(buffer, sizeof(buffer)) );
        assert( memfx_load_file(mem, filename) );
        assert( memfx_append(mem, "\x00", 1) );  // Add null terminator
        printf("%s\n", (char*)mem->buf);
        memfx_deinit(mem);

        remove(filename);
    }
}

int main(void)
{
    test_global_tools();
    test_memory_object();
    test_fixed_buffer_memory_object();

    return 0;
}

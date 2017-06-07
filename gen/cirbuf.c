#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "minmax.h"
#include "intutil.h"
#include "cirbuf.h"

//------------------------------------------------------------------------------
static
size_t calc_map_buf_size(size_t sizereq)
{
    size_t size = sizereq;
    ++size;  // Because the available size will be less (1) then the actual buffer size.

#if defined(CIRBUF_USE_MEMMAP) && defined(__linux__)
    static long page_size = 0;
    if( !page_size ) page_size = sysconf(_SC_PAGE_SIZE);
    assert( page_size );

    assert( intutil_is_pow2(page_size) );
    size = intutil_ceil_mul_pow2(size, page_size);
#elif defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
    static long alloc_granu = 0;
    if( !alloc_granu )
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        alloc_granu = sysinfo.dwAllocationGranularity;
    }
    assert( alloc_granu );

    assert( intutil_is_pow2(alloc_granu) );
    size = intutil_ceil_mul_pow2(size, alloc_granu);
#else
    // Nothing to do.
#endif

    return intutil_ceil_pow2(size);  // We need the size to be one of the power series of 2
                                     // for optimisation reason.
}
//------------------------------------------------------------------------------
#if defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
static
uint8_t* win32_query_map_addr(size_t halfsize)
{
    uint8_t *addr = VirtualAlloc(NULL, halfsize << 1, MEM_RESERVE, PAGE_NOACCESS);
    if( addr ) VirtualFree(addr, 0, MEM_RELEASE);
    return addr;
}
#endif
//------------------------------------------------------------------------------
#if defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
static
uint8_t* win32_bind_map_addr(uint8_t *addr, size_t halfsize)
{
    HANDLE   file  = NULL;
    uint8_t *addr1 = NULL;
    uint8_t *addr2 = NULL;

    bool success = false;
    do
    {
        if( !addr ) break;

        file = CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 halfsize << 1,
                                 NULL);
        if( !file ) break;

        addr1 = MapViewOfFileEx(file,
                                FILE_MAP_ALL_ACCESS,
                                0,
                                0,
                                halfsize,
                                addr + 0);
        if( !addr1 ) break;
        assert( addr1 == addr + 0 );

        addr2 = MapViewOfFileEx(file,
                                FILE_MAP_ALL_ACCESS,
                                0,
                                0,
                                halfsize,
                                addr + halfsize);
        if( !addr2 ) break;
        assert( addr2 == addr + halfsize );

        success = true;
    } while(false);

    if( !success )
    {
        if( addr1 ) UnmapViewOfFile(addr1);
        if( addr2 ) UnmapViewOfFile(addr2);
        addr = NULL;
    }

    if( file )
        CloseHandle(file);

    return addr;
}
#endif
//------------------------------------------------------------------------------
static
void* alloc_map_buf(size_t halfsize)
{
#if defined(CIRBUF_USE_MEMMAP) && defined(__linux__)
    uint8_t *addr = MAP_FAILED;
    int      fd   = -1;

    bool success = false;
    do
    {
        char filename[] = "/dev/shm/circular-buffer-XXXXXX";
        fd = mkstemp(filename);
        unlink(filename);
        if( fd < 0 ) break;

        if( ftruncate(fd, halfsize) ) break;

        addr = mmap(NULL,
                    halfsize << 1,
                    PROT_NONE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1,
                    0);
        if( addr == MAP_FAILED ) break;

        uint8_t *addr1 = mmap(addr + 0,
                              halfsize,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_FIXED,
                              fd,
                              0);
        if( addr1 == MAP_FAILED ) break;
        assert( addr1 == addr + 0 );

        uint8_t *addr2 = mmap(addr + halfsize,
                              halfsize,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_FIXED,
                              fd,
                              0);
        if( addr2 == MAP_FAILED ) break;
        assert( addr2 == addr + halfsize );

        success = true;
    } while(false);

    if( !success && addr != MAP_FAILED )
    {
        munmap( addr, halfsize << 1 );
        addr = MAP_FAILED;
    }

    if( fd >= 0 )
        close(fd);

    return addr == MAP_FAILED ? NULL : addr;
#elif defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
    uint8_t *addr = NULL;

    int retrycnt = 1024;
    while( !addr && retrycnt-- )
    {
        addr = ( addr = win32_query_map_addr(halfsize) )?
               ( win32_bind_map_addr(addr, halfsize) ):
               ( NULL );
    }

    return addr;
#else
    return malloc( halfsize << 1 );
#endif
}
//------------------------------------------------------------------------------
static
void free_map_buf(uint8_t *addr, size_t halfsize)
{
    assert( addr && halfsize );

#if defined(CIRBUF_USE_MEMMAP) && defined(__linux__)
    munmap( addr, halfsize << 1 );
#elif defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
    UnmapViewOfFile( addr + 0 );
    UnmapViewOfFile( addr + halfsize );
#else
    free(addr);
#endif
}
//------------------------------------------------------------------------------
void cirbuf_init(cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Constructor.
     */
    assert( self );
    memset(self, 0, sizeof(*self));
}
//------------------------------------------------------------------------------
void cirbuf_deinit(cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Destructor.
     */
    assert( self );
    cirbuf_dealloc(self);
}
//------------------------------------------------------------------------------
size_t cirbuf_alloc(cirbuf_t *self, size_t size)
{
    /**
     * @memberof cirbuf_t
     * @brief Allocate circular buffer.
     *
     * @param self Object instance.
     * @param size The size of the buffer required.
     * @return The actual size of buffer that can be use,
     *         it will be greater or equal to the size required;
     *         or ZERO if allocation failed.
     */
    assert( self );

    if( self->buf )
        cirbuf_dealloc(self);

    size = calc_map_buf_size(size);
    if( !size ) return 0;

    self->buf = alloc_map_buf(size);
    if( !self->buf ) return 0;

    self->size     = size;
    self->sizemask = size - 1;

    return size - 1;
}
//------------------------------------------------------------------------------
void cirbuf_dealloc(cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Deallocate circular buffer.
     */
    assert( self );

    if( self->buf )
    {
        free_map_buf(self->buf, self->size);
        memset(self, 0, sizeof(*self));
    }
}
//------------------------------------------------------------------------------
void cirbuf_clear(cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Clear all data in buffer.
     */
    assert( self );

    self->rpos = 0;
    self->wpos = 0;
}
//------------------------------------------------------------------------------
size_t cirbuf_get_datasize(const cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Get data size.
     *
     * @param self Object instance.
     * @return The size of data in buffer.
     */
    assert( self );

    return ( self->rpos > self->wpos )?
           ( self->size - ( self->rpos - self->wpos ) ):
           ( self->wpos - self->rpos );
}
//------------------------------------------------------------------------------
size_t cirbuf_get_freesize(const cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Get buffer free size.
     *
     * @param self Object instance.
     * @return The available size of buffer that does not be used currently.
     */
    assert( self );

    return ( self->rpos > self->wpos )?
           ( self->rpos - self->wpos - 1 ):
           ( self->sizemask - ( self->wpos - self->rpos ) );
}
//------------------------------------------------------------------------------
const void* cirbuf_get_read_buf(const cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Get data buffer position, so that user can read data directly with it.
     *
     * @param self Object instance.
     * @return The start pointer of buffer that have data to read; or
     *         NULL if the buffer had not be allocated.
     *
     * @remarks
     *     @li The circular buffer must be allocated before calling this function.
     *     @li User can call cirbuf_t::cirbuf_get_datasize to get
     *         how many bytes of data can be read.
     */
    assert( self );
    assert( self->buf );
    return self->buf + self->rpos;
}
//------------------------------------------------------------------------------
void* cirbuf_get_write_buf(cirbuf_t *self)
{
    /**
     * @memberof cirbuf_t
     * @brief Get available buffer position, so that user can write data directly with it.
     *
     * @param self Object instance.
     * @return The start pointer of buffer that be available to fill data.
     *
     * @remarks
     *     @li The circular buffer must be allocated before calling this function.
     *     @li User can call cirbuf_t::cirbuf_get_restsize to get
     *         the maximum bytes of data that can be filled in.
     */
    assert( self );
    assert( self->buf );
    return self->buf + self->wpos;
}
//------------------------------------------------------------------------------
size_t cirbuf_commit_read(cirbuf_t *self, size_t size)
{
    /**
     * @memberof cirbuf_t
     * @brief Notify the object that how many data has been read.
     *
     * @param self Object instance.
     * @param size Bytes of data that has been read.
     * @return Bytes of data that has been removed from the buffer.
     *         Normally, the return value will be equal to the input,
     *         Except the input size had been greater then
     *         available size of data in the buffer.
     */
    assert( self );

    size_t datasize = cirbuf_get_datasize(self);
    size = MIN( size, datasize );

    self->rpos += size;
    self->rpos &= self->sizemask;

    return size;
}
//------------------------------------------------------------------------------
size_t cirbuf_commit_write(cirbuf_t *self, size_t size)
{
    /**
     * @memberof cirbuf_t
     * @brief Notify the object that how many data has been filled in.
     *
     * @param self Object instance.
     * @param size Bytes of data that has filled in the buffer.
     * @return Bytes of data that has been append to the buffer.
     *         Normally, the return value will be equal to the input,
     *         Except the input size had been greater then
     *         available size of buffer.
     */
    assert( self );

    size_t bufsize = cirbuf_get_freesize(self);
    size = MIN( size, bufsize );

#if defined(CIRBUF_USE_MEMMAP) && defined(__linux__)
    // Nothing to do.
#elif defined(CIRBUF_USE_MEMMAP) && defined(_WIN32)
    // Nothing to do.
#else
    size_t wm = self->size;
    size_t wo = self->wpos;
    size_t zo = size;
    size_t wu = wo + self->size;
    size_t zu = ( wo + zo > self->size )?( self->size - wo ):( zo );
    size_t wl = 0;
    size_t zl = zo - zu;

    memcpy( self->buf + wu , self->buf + wo , zu );
    memcpy( self->buf + wl , self->buf + wm , zl );
#endif

    self->wpos += size;
    self->wpos &= self->sizemask;

    return size;
}
//------------------------------------------------------------------------------
size_t cirbuf_read(cirbuf_t *self, void *buf, size_t size)
{
    /**
     * @memberof cirbuf_t
     * @brief Read data.
     *
     * @param self Object instance.
     * @param buf  A buffer to receive data.
     * @param size Size of data required.
     * @return The actual size of data that has been filled to the output buffer.
     */
    assert( self );
    assert( self->buf && buf );

    size_t datasize = cirbuf_get_datasize(self);
    size = MIN( size, datasize );

    memcpy(buf, cirbuf_get_read_buf(self), size);
    return cirbuf_commit_read(self, size);
}
//------------------------------------------------------------------------------
size_t cirbuf_write(cirbuf_t *self, const void *data, size_t size)
{
    /**
     * @memberof cirbuf_t
     * @brief Write data.
     *
     * @param self Object instance.
     * @param data The data to write.
     * @param size Size of input data.
     * @return The actual size of data that has been filled in to the buffer.
     */
    assert( self );
    assert( self->buf && data );

    size_t bufsize = cirbuf_get_freesize(self);
    size = MIN( size, bufsize );

    memcpy(cirbuf_get_write_buf(self), data, size);
    return cirbuf_commit_write(self, size);
}
//------------------------------------------------------------------------------

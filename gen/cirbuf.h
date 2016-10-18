/**
 * @file
 * @brief     Circular buffer.
 * @details   Circular buffer implementation.
 * @author    王文佑
 * @date      2016.03.13
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CIRBUF_H_
#define _GEN_CIRBUF_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def CIRBUF_USE_MEMMAP
 *
 * Use memory mapping technique on the circular buffer
 * if the environment support it.
 */
#define CIRBUF_USE_MEMMAP

/**
 * @class cirbuf_t
 * @brief Circular buffer.
 */
typedef struct cirbuf_t
{
    // WARNING : All variables are private!

    // Buffer.
    uint8_t *buf;
    size_t   size;  // Must be power series of 2.

    // Pre calculated parameters for more calculate efficiency.
    size_t sizemask;  // Mask to fast calculate the modulus of buffer size.
                      // Equivalent to ( buffer_size - 1 ).

    // Access position.
    size_t rpos;
    size_t wpos;

} cirbuf_t;

void cirbuf_init  (cirbuf_t *self);
void cirbuf_deinit(cirbuf_t *self);

size_t cirbuf_alloc  (cirbuf_t *self, size_t size);
void   cirbuf_dealloc(cirbuf_t *self);

void cirbuf_clear(cirbuf_t *self);

size_t cirbuf_get_datasize(const cirbuf_t *self);
size_t cirbuf_get_freesize(const cirbuf_t *self);

const void* cirbuf_get_read_buf (const cirbuf_t *self);
void*       cirbuf_get_write_buf(      cirbuf_t *self);

size_t cirbuf_commit_read (cirbuf_t *self, size_t size);
size_t cirbuf_commit_write(cirbuf_t *self, size_t size);

size_t cirbuf_read (cirbuf_t *self, void *buf, size_t size);
size_t cirbuf_write(cirbuf_t *self, const void *data, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of cirbuf_t
class TCirBuf : protected cirbuf_t
{
public:
    TCirBuf()  { cirbuf_init  (this); }  ///< @see cirbuf_t::cirbuf_init
    ~TCirBuf() { cirbuf_deinit(this); }  ///< @see cirbuf_t::cirbuf_deinit

public:
    size_t Allocate(size_t size)  { return cirbuf_alloc  (this, size); }    ///< @see cirbuf_t::cirbuf_alloc
    void   Deallocate()           {        cirbuf_dealloc(this); }          ///< @see cirbuf_t::cirbuf_dealloc

    void Clear() { cirbuf_clear(this); }    ///< @see cirbuf_t::cirbuf_clear

    size_t GetDataSize() const { return cirbuf_get_datasize(this); }    ///< @see cirbuf_t::cirbuf_get_datasize
    size_t GetFreeSize() const { return cirbuf_get_freesize(this); }    ///< @see cirbuf_t::cirbuf_get_freesize

    const void* GetReadBuffer() const { return cirbuf_get_read_buf (this); }    ///< @see cirbuf_t::cirbuf_get_read_buf
    void*       GetWriteBuffer()      { return cirbuf_get_write_buf(this); }    ///< @see cirbuf_t::cirbuf_get_write_buf

    size_t CommitRead (size_t size) { return cirbuf_commit_read (this, size); }     ///< @see cirbuf_t::cirbuf_commit_read
    size_t CommitWrite(size_t size) { return cirbuf_commit_write(this, size); }     ///< @see cirbuf_t::cirbuf_commit_write

    size_t Read (void *buf, size_t size)        { return cirbuf_read (this, buf, size); }   ///< @see cirbuf_t::cirbuf_read
    size_t Write(const void *data, size_t size) { return cirbuf_write(this, data, size); }  ///< @see cirbuf_t::cirbuf_write

};

#endif

#endif

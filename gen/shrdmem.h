/**
 * @file
 * @brief     Shared memory object.
 * @details   Encapsulation of system wide shared memory operation details.
 * @author    王文佑
 * @date      2014.01.23
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_SHRDMEM_H_
#define _GEN_SHRDMEM_H_

#ifdef __cplusplus
#include <string>
#endif

#include "type.h"
#include "restrict.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
    #define SHRDMEM_SHRDPATH "/dev/shm/"  // File path to place shared objects.
#endif

/// @class shrdmem_t
/// @brief Shared memory.
typedef struct shrdmem_t
{

    // Private members
#if   defined(__linux__)
    int     fd;
    bool    remove_file_when_close;
#elif defined(_WIN32)
    void   *hmap;
#else
    #error No implementation on this platform!
#endif

    // Public
    size_t  size;  ///< @brief 資料緩衝區大小(Read Only)。
                   ///< @warning 因為某些緣故，我們提供使用者直接存取這個變數的方式，而不是以函式為之。
                   ///<          但這個變數為唯讀變數，請勿直接更改其值。

    byte_t *buf;   ///< @brief 共用資料緩衝區。

} shrdmem_t;

void shrdmem_init  (shrdmem_t* RESTRICT self);
void shrdmem_deinit(shrdmem_t* RESTRICT self);

shrdmem_t* shrdmem_create             (void);
shrdmem_t* shrdmem_create_open        (const char* RESTRICT name, size_t size, bool fail_if_existed);
shrdmem_t* shrdmem_create_open_existed(const char* RESTRICT name);
void shrdmem_release                  (shrdmem_t*  RESTRICT self);
void shrdmem_release_s                (shrdmem_t** RESTRICT self);

void shrdmem_set_zeros   (      shrdmem_t* RESTRICT self);
bool shrdmem_open        (      shrdmem_t* RESTRICT self, const char* RESTRICT name, size_t size, bool fail_if_existed);
bool shrdmem_open_existed(      shrdmem_t* RESTRICT self, const char* RESTRICT name);
void shrdmem_close       (      shrdmem_t* RESTRICT self);
bool shrdmem_is_opened   (const shrdmem_t* RESTRICT self);

bool shrdmem_query_existed(const char* RESTRICT name);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ Wrapper of @ref shrdmem_t
class TShrdMem : protected shrdmem_t
{
public:
    TShrdMem (){ shrdmem_init  (this); }
    ~TShrdMem(){ shrdmem_deinit(this); }
private:
    TShrdMem(const TShrdMem&);             // Not allowed to use
    TShrdMem& operator=(const TShrdMem&);  // Not allowed to use

public:
    size_t        Size() const { return size; }  ///< Get buffer size.
    const byte_t* Buf () const { return buf; }   ///< Get data buffer.
    byte_t*       Buf ()       { return buf; }   ///< Get data buffer.

    void SetZeros   ()                          {        shrdmem_set_zeros   (this); }                                     ///< @see shrdmem_t::shrdmem_set_zeros
    bool Open       (const std::string &Name, size_t Size, bool FailIfExisted)
                                                { return shrdmem_open        (this, Name.c_str(), Size, FailIfExisted); }  ///< @see shrdmem_t::shrdmem_open
    bool OpenExisted(const std::string &Name)   { return shrdmem_open_existed(this, Name.c_str()); }                       ///< @see shrdmem_t::shrdmem_open_existed
    void Close      ()                          {        shrdmem_close       (this); }                                     ///< @see shrdmem_t::shrdmem_close
    bool IsOpened   () const                    { return shrdmem_is_opened   (this); }                                     ///< @see shrdmem_t::shrdmem_is_open

    static bool QueryExisted(const std::string &Name){ return shrdmem_query_existed(Name.c_str()); }  ///< @see shrdmem_t::shrdmem_query_existed

};

#endif

#endif

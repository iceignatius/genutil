/**
 * @file
 * @brief     Memory object
 * @details   To support simply memory buffer objects to handle the details of buffer operation.
 * @author    王文佑
 * @date      2014.01.20
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_MEMOBJ_H_
#define _GEN_MEMOBJ_H_

#ifdef __cplusplus
#include <new>
#include <string>
#endif

#include "type.h"
#include "inline.h"
#include "restrict.h"

//----------------------------------------
//---- Global Setting and Tools ----------
//----------------------------------------

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
    #define MEMOJB_ARG_DEFAULT(value) =value
#else
    #define MEMOJB_ARG_DEFAULT(value)
#endif

#define MEMOJB_BLOCKSZ_SMALL sizeof(int)  // 緩衝區尺寸將為此數的倍數
#define MEMOJB_BLOCKSZ_LARGE 512          // 緩衝區塊配置的大單位尺寸
STATIC_ASSERT( MEMOJB_BLOCKSZ_LARGE % MEMOJB_BLOCKSZ_SMALL == 0 );

void* memfind (const void* src, size_t srcsz, const void* pattern, size_t patsz);
void* memrfind(const void* src, size_t srcsz, const void* pattern, size_t patsz);

unsigned memfindcount(const void* src, size_t srcsz, const void* pattern, size_t patsz);
size_t   memfindreplace(void*       dest,
                        size_t      destsz,
                        const void* src,
                        size_t      srcsz,
                        const void* pattern,
                        size_t      patsz,
                        const void* target,
                        size_t      tarsz);

size_t memobj_calc_recommended_size(size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

//----------------------------------------
//---- Memory Buffer Object --------------
//----------------------------------------

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @class mem_t
 * @brief Memory buffer object
 *
 * @attention
 * Some rules need to be followed:
 *     @li Functions with prefix "mem_init" are using to initialize a memory object with the buffer of mem_t which is already allocated,
 *         and it needs to call "mem_deinit" to de-initialize the object.
 *     @li Functions with prefix "mem_create" are using to allocate and initialize a memory object,
 *         and needs to call "mem_release" to release the object.
 */
typedef struct mem_t
{
    // Private

    size_t  size_total;  // 緩衝區的實際大小。
                         // 這個數值由本類別所私有使用，一般使用者請勿使用及變更此變數。

    // Public

    size_t  size;        ///< @brief 緩衝區被使用的大小(Read Only)。
                         ///< @warning 因為某些緣故，我們提供使用者直接存取這個變數的方式，而不是以函式為之。
                         ///<          但這個變數為唯讀變數，請勿直接更改其值；
                         ///<          欲變動緩衝區大小，請使用 mem_t::mem_resize 函式。

    byte_t *buf;         ///< 資料緩衝區。

} mem_t;

void   mem_init       (mem_t* RESTRICT self, size_t size MEMOJB_ARG_DEFAULT(0));
void   mem_init_import(mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size);
void   mem_init_clone (mem_t* RESTRICT self, const mem_t* RESTRICT src);
void   mem_init_move  (mem_t* RESTRICT self, mem_t* RESTRICT src);
void   mem_deinit     (mem_t* RESTRICT self);

mem_t* mem_create          (size_t size MEMOJB_ARG_DEFAULT(0));
mem_t* mem_create_import   (const void*  RESTRICT buffer, size_t size);
mem_t* mem_create_clone    (const mem_t* RESTRICT src);
mem_t* mem_create_move     (mem_t*       RESTRICT src);
mem_t* mem_create_load_file(const char*  RESTRICT filename);
void   mem_release         (mem_t*       RESTRICT self);
void   mem_release_s       (mem_t**      RESTRICT self);

void        mem_set_zeros(      mem_t* RESTRICT self);
bool        mem_resize   (      mem_t* RESTRICT self, size_t size);
INLINE void mem_clear    (      mem_t* RESTRICT self) { mem_resize(self,0); }  ///< @memberof mem_t @brief Reset buffer.
bool        mem_import   (      mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size);
bool        mem_append   (      mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size);
void        mem_move_from(      mem_t* RESTRICT self, mem_t* RESTRICT src);
void        mem_pop_front(      mem_t* RESTRICT self, size_t popsz);

int         mem_compare     (const mem_t* RESTRICT self, const mem_t* RESTRICT tar);
const void* mem_find        (const mem_t* RESTRICT self, const mem_t* RESTRICT pattern);
const void* mem_rfind       (const mem_t* RESTRICT self, const mem_t* RESTRICT pattern);
unsigned    mem_find_count  (const mem_t* RESTRICT self, const mem_t* RESTRICT pattern);
bool        mem_find_replace(      mem_t* RESTRICT self, const mem_t* RESTRICT pattern, const mem_t* RESTRICT target);

bool        mem_save_file(const mem_t* RESTRICT self, const char* RESTRICT filename);
bool        mem_load_file(      mem_t* RESTRICT self, const char* RESTRICT filename);
bool        mem_save_text(const mem_t* RESTRICT self, const char* RESTRICT filename);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/**
 * @brief C++ wrapper of @ref mem_t
 */
class TMem : protected mem_t
{
public:
    TMem(size_t Size=0)                     { mem_init       (this, Size); }            ///< @see mem_t::mem_init
    TMem(const void *Buffer, size_t Size)   { mem_init_import(this, Buffer, Size); }    ///< @see mem_t::mem_init_import
    TMem(const TMem &Src)                   { mem_init_clone (this, &Src); }            ///< @see mem_t::mem_init_clone
#if __cplusplus >= 201103L
    TMem(TMem &&Src)                        { mem_init_move  (this, &Src); }            ///< @see mem_t::mem_init_move
#endif
    ~TMem()                                 {      mem_deinit(this); }                  ///< @see mem_t::mem_deinit

    TMem& operator=(const TMem &Src)        { if( !mem_import(this, Src.buf, Src.size) ) throw std::bad_alloc(); return *this; }
#if __cplusplus >= 201103L
    TMem& operator=(TMem &&Src)             {      mem_move_from(this, &Src); return *this; }
#endif

public:
    size_t        Size() const { return size; }  ///< Get buffer size.
    const byte_t* Buf () const { return buf; }   ///< Get data buffer.
    byte_t*       Buf ()       { return buf; }   ///< Get data buffer.

    void SetZeros()                                 {      mem_set_zeros(this); }  ///< @see mem_t::mem_set_zeros
    void Resize  (size_t Size)                      { if( !mem_resize   (this, Size)              ) throw std::bad_alloc(); }  ///< @see mem_t::mem_resize
    void Clear   ()                                 {      mem_clear    (this); }  ///< @see mem_t::mem_clear
    void Import  (const void *Buffer, size_t Size)  { if( !mem_import   (this, Buffer, Size)      ) throw std::bad_alloc(); }  ///< @see mem_t::mem_import
    void Append  (const void *Buffer, size_t Size)  { if( !mem_append   (this, Buffer, Size)      ) throw std::bad_alloc(); }  ///< @see mem_t::mem_append
    void Append  (const TMem &Src)                  { if( !mem_append   (this, Src.buf, Src.size) ) throw std::bad_alloc(); }  ///< @see mem_t::mem_append
    void MoveFrom(TMem &Src)                        {      mem_move_from(this, &Src); }
    void PopFront(size_t PopSize)                   {      mem_pop_front(this, PopSize); }

    int         Compare    (const TMem &target)  const { return mem_compare(this, &target); }      ///< @see mem_t::mem_compare
    const void* Find       (const TMem &pattern) const { return mem_find(this, &pattern); }        ///< @see mem_t::mem_find
    const void* RFind      (const TMem &pattern) const { return mem_rfind(this, &pattern); }       ///< @see mem_t::mem_rfind
    unsigned    FindCount  (const TMem &pattern) const { return mem_find_count(this, &pattern); }  ///< @see mem_t::mem_find_count
    void        FindReplace(const TMem &pattern, const TMem &target) { if( !mem_find_replace(this, &pattern, &target) ) throw std::bad_alloc(); }  ///< @see mem_t::mem_find_replace

    bool SaveFile(const std::string &Filename) const { return mem_save_file(this, Filename.c_str()); }  ///< @see mem_t::mem_save_file
    bool LoadFile(const std::string &Filename)       { return mem_load_file(this, Filename.c_str()); }  ///< @see mem_t::mem_load_file
    bool SaveText(const std::string &Filename) const { return mem_save_text(this, Filename.c_str()); }  ///< @see mem_t::mem_save_text

public:
    bool operator==(const TMem &tar) { return 0 == Compare(tar); }  ///< Contents comparison encapsulation.
    bool operator!=(const TMem &tar) { return 0 != Compare(tar); }  ///< Contents comparison encapsulation.
    bool operator< (const TMem &tar) { return 0 >  Compare(tar); }  ///< Contents comparison encapsulation.
    bool operator> (const TMem &tar) { return 0 <  Compare(tar); }  ///< Contents comparison encapsulation.

};

#endif

//----------------------------------------
//---- Memory Object (Fixed Buffer) ------
//----------------------------------------

/*
 * 這個模組是專為 Heap 空間受限的 Embedded System 等使用環境所設計，
 * 使用上類似於使用變動緩衝空間的 mem 模組，但功能更受限。
 * memfx 模組僅使用初始傳入之資料緩衝區做使用，不觸及任何浮動空間的操作。
 */

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @class memfx_t
 * @brief Fixed buffer memory object.
 *
 * @remarks
 * 雖然以外部傳入之固定緩衝區做為操作使用的物件，理應不需在結束時進行清理釋放等行為；
 * 但我們仍強烈建議使用者在結束使用 memfx 物件後應當呼叫 memfx_deinit 這個函式，這有兩個好處：
 * 一是能夠維持相似用途物件在使用上的一致性，二是在物件類型需要更換的時候能夠較為順利。
 */
typedef struct memfx_t
{
    // Private

    size_t size_total;  // 整個緩衝區的容量，包含這裡所定義的標頭資訊以及後部的資料緩衝空間。
                        // 這個數值由本類別所私有使用，一般使用者請勿使用及變更此變數。

    // Public

    size_t size;        ///< @brief 後部資料緩衝區當前被使用的大小。
                        ///< @warning 因為某些緣故，我們提供使用者直接存取這個變數的方式，而不是以函式為之。
                        ///<          但這個變數為唯讀變數，請勿直接更改其值；
                        ///<          欲變動緩衝區大小，請使用 memfx_t::memfx_resize 函式。

    byte_t buf[];       ///< 資料緩衝區。

} memfx_t, memfx_header_t;

STATIC_ASSERT( sizeof(memfx_header_t) == offsetof(memfx_t,buf) );

memfx_t*    memfx_init  (byte_t* RESTRICT buffer, size_t bufsz, size_t initsz MEMOJB_ARG_DEFAULT(0));
INLINE void memfx_deinit(memfx_t* RESTRICT self) {}  ///< @memberof memfx_t @brief Destructor.

void        memfx_set_zeros(      memfx_t* RESTRICT self);
bool        memfx_resize   (      memfx_t* RESTRICT self, size_t size);
INLINE void memfx_clear    (      memfx_t* RESTRICT self) { memfx_resize(self,0); }  ///< @memberof memfx_t @brief Reset buffer.
bool        memfx_import   (      memfx_t* RESTRICT self, const void* RESTRICT buffer, size_t size);
bool        memfx_append   (      memfx_t* RESTRICT self, const void* RESTRICT buffer, size_t size);
void        memfx_pop_front(      memfx_t* RESTRICT self, size_t popsz);

int         memfx_compare   (const memfx_t* RESTRICT self, const memfx_t* RESTRICT tar);
const void* memfx_find      (const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern);
const void* memfx_rfind     (const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern);
unsigned    memfx_find_count(const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern);

bool        memfx_save_file(const memfx_t* RESTRICT self, const char* RESTRICT filename);
bool        memfx_load_file(      memfx_t* RESTRICT self, const char* RESTRICT filename);
bool        memfx_save_text(const memfx_t* RESTRICT self, const char* RESTRICT filename);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

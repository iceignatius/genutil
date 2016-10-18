/**
 * @file
 * @brief     Memory pool
 * @details   To support user space memory pool objects to help some purpose, such like processes communication.
 * @author    王文佑
 * @date      2014.05.09
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 *
 * @note 模組設計原則：
 *     @li 本模組主要設計用途為如使用共用記憶體進行行程間通信時需要做資料管理的場合。
 *     @li 各物件內部所管理的記憶體位置皆以偏移量紀錄，以利不同地址空間的應用(偏移量零
 *         保留用來標示無效的偏移量)。
 *     @li 考量到可能有其它行程共同使用共用資料的狀況，本功能集的所有公開物件皆不會主
 *         動初使化物件內容，也不會主動銷毀物件與資料，使用者必須自行判斷需要執行上述
 *         工作的時機。
 */
#ifndef _GEN_MEMPOOL_H_
#define _GEN_MEMPOOL_H_

#include "type.h"
#include "inline.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @class mempool_t
 * @brief Memory pool
 */
typedef struct mempool_t mempool_t;

// 緩衝區設定
mempool_t* mempool_init(void* buffer, size_t size);

// 記憶體位址與偏移量轉換
uint64_t mempool_offset_from_addr(const mempool_t* pool, const void* bufaddr);
void*    mempool_offset_to_addr  (const mempool_t* pool, uint64_t    bufoff);

// 記憶體資訊查詢
size_t mempool_get_pool_size  (const mempool_t* pool);
size_t mempool_get_free_size  (const mempool_t* pool);
size_t mempool_get_buffer_size(const mempool_t* pool, const void* bufaddr);
void*  mempool_get_inuse_next (const mempool_t* pool, const void* bufaddr);

INLINE void* mempool_get_inuse_first(const mempool_t* pool)
{
    /**
     * @memberof mempool_t
     * @brief Get first allocated buffer.
     * @param pool Object instance.
     * @return The first allocated buffer; or NULL if there have no allocated buffer.
     */
    return mempool_get_inuse_next(pool,NULL);
}

// 記憶體分配
void* mempool_allocate  (mempool_t* pool, size_t size);
void  mempool_deallocate(mempool_t* pool, void* bufaddr);
void* mempool_reallocate(mempool_t* pool, void* bufaddr, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of @ref mempool_t
class TMemPool
{
private:
    TMemPool();                             // Not allowed to use
    TMemPool(const TMemPool&);              // Not allowed to use
    TMemPool& operator=(const TMemPool&);   // Not allowed to use

public:
    static TMemPool* Initialize(void* Buffer, size_t Size) { return (TMemPool*)mempool_init(Buffer, Size); }  ///< @see mempool_t::mempool_init

public:
    uint64_t OffsetFromAddr(const void* BufAddr) const { return mempool_offset_from_addr((const mempool_t*)this, BufAddr); }  ///< @see mempool_t::mempool_offset_from_addr
    void*    OffsetToAddr  (uint64_t    BufOff ) const { return mempool_offset_to_addr  ((const mempool_t*)this, BufOff ); }  ///< @see mempool_t::mempool_offset_to_addr

public:
    size_t GetPoolSize  ()                    const { return mempool_get_pool_size  ((const mempool_t*)this); }           ///< @see mempool_t::mempool_get_pool_size
    size_t GetFreeSize  ()                    const { return mempool_get_free_size  ((const mempool_t*)this); }           ///< @see mempool_t::mempool_get_free_size
    size_t GetBufferSize(const void* BufAddr) const { return mempool_get_buffer_size((const mempool_t*)this, BufAddr); }  ///< @see mempool_t::mempool_get_buffer_size
    void*  GetInuseNext (const void* BufAddr) const { return mempool_get_inuse_next ((const mempool_t*)this, BufAddr); }  ///< @see mempool_t::mempool_get_inuse_next
    void*  GetInuseFirst()                    const { return mempool_get_inuse_first((const mempool_t*)this); }           ///< @see mempool_t::mempool_get_inuse_first

public:
    void* Allocate  (size_t Size                ) { return mempool_allocate  ((mempool_t*)this, Size); }           ///< @see mempool_t::mempool_allocate
    void  Deallocate(void*  BufAddr             ) { return mempool_deallocate((mempool_t*)this, BufAddr); }        ///< @see mempool_t::mempool_deallocate
    void* Reallocate(void*  BufAddr, size_t Size) { return mempool_reallocate((mempool_t*)this, BufAddr, Size); }  ///< @see mempool_t::mempool_reallocate

};

#endif

#endif

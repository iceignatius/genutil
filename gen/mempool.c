#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "type.h"
#include "mempool.h"

// Memory block node align size
static const size_t node_alignsz = 32;  // WARNING : This value must be the power series of 2.

// Memory block node
#pragma pack(push,8)
typedef struct node_t
{
    uint64_t  size;         // Size of this node, include the user data buffer.
                            // This value will be ZERO if this node is the last node of the list.
    uint64_t  prev;         // Offset between the head of this node and the head of previous node.
                            // This value will be ZERO if this node is the last node of the list.
    bool8_t   inuse;        // The flag to mark if this node have been allocated
                            // to the client user.
    byte_t    reserved[7];
    uint64_t  usersize;     // Size of the data buffer assigned by the user.
    byte_t    userdata[];   // User data buffer.
} node_t, node_header_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(node_t) == 32 );

// Memory pool header
#pragma pack(push,8)
typedef struct mempool_t
{
    uint64_t size;          // Total size of this pool.
    byte_t   reserved[24];  // Reserved use, and needs to be zeros.
    node_t   root[];        // The first node of this pool.
} mempool_t, mempool_header_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(mempool_t) == 32 );

//------------------------------------------------------------------------------
//---- Tools -------------------------------------------------------------------
//------------------------------------------------------------------------------
INLINE
size_t calc_align_size(size_t size)
{
    return ( size + node_alignsz - 1 ) & ~( node_alignsz - 1 );
}
//------------------------------------------------------------------------------
//---- Pool Node and List ------------------------------------------------------
//------------------------------------------------------------------------------
static
node_t* node_from_useraddr(void* bufaddr)
{
    /*
     * 從使用者資料緩衝區的位址計算取得該資料緩衝區所屬的節點；失敗時傳回 NULL。
     */
    if( !bufaddr || (size_t)bufaddr < sizeof(node_t) ) return NULL;
    return (node_t*)( (byte_t*)bufaddr - sizeof(node_t) );
}
//------------------------------------------------------------------------------
static
node_t* node_get_next(node_t *node)
{
    /*
     * 取得傳入節點在串列上的下一個節點；若無下一個節點則傳回 NULL。
     */
    if( !node || !node->size ) return NULL;
    node = (node_t*)( (byte_t*)node + node->size );
    return ( node->size )?( node ):( NULL );
}
//------------------------------------------------------------------------------
static
node_t* node_get_prev(node_t *node)
{
    /*
     * 取得傳入節點在串列上的前一個節點；若無前一個節點則傳回 NULL。
     */
    if( !node || !node->prev ) return NULL;
    return (node_t*)( (byte_t*)node - node->prev );
}
//------------------------------------------------------------------------------
static
node_t* node_separate(node_t *node, size_t reqsz)
{
    /*
     * 將一個節點切割為兩個彼此相連的節點。
     * @param node  傳入欲做切割的節點。
     * @param reqsz 傳入切割後的第一個節點所應具有的最小大小。
     * @return 成功時傳回切割後的第二個節點指標，失敗時傳回 NULL。
     */
    const size_t node_size_min = calc_align_size(sizeof(node_t));
    size_t       blocksz       = calc_align_size(reqsz);
    node_t      *node_left     = node;
    node_t      *node_right    = node_get_next(node_left);
    node_t      *node_centre;

    assert( node_size_min >= sizeof(node_t) );
    assert( blocksz       >= reqsz          );

    if( !node_left ) return NULL;
    if( blocksz + node_size_min > node_left->size ) return NULL;

    node_centre = (node_t*)( (byte_t*)node_left + blocksz );
    memset(node_centre, 0, sizeof(node_t));

    node_centre->size = node_left->size - blocksz;
    node_centre->prev = blocksz;
    node_left  ->size = blocksz;

    if( node_right ) node_right->prev = node_centre->size;

    return node_centre;
}
//------------------------------------------------------------------------------
static
void node_merge_next(node_t *node)
{
    /*
     * 將傳入的節點與下一個節點合併，如果下一個節點不是串列的結尾節點的話。
     */
    node_t *node_left   = node;
    node_t *node_centre = node_get_next(node_left);
    node_t *node_right  = node_get_next(node_centre);

    if( !node_centre ) return;

    if( !node_left ) return;
    node_left->size += node_centre->size;

    if( !node_right ) return;
    node_right->prev = node_left->size;
}
//------------------------------------------------------------------------------
static
node_t* nodelist_init(void* buffer, size_t size)
{
    /*
     * 將一塊資料空間的內容初始化為一個單一節點(扣除必要的結尾標示節點以外)的串列。
     * @param buffer 傳入一個供應給串列使用的資料空間。
     * @param size   傳入 buffer 的大小。
     * @return 成功時傳回初始化完成的串列指標，該位址與傳入的 buffer 位址相同，
     *         即除了表面型態的不同外，buffer 與串列是同一個物件。
     *         當 buffer 的空間不足以做為串列使用時會傳回 NULL。
     */
    const size_t node_size_min = calc_align_size(sizeof(node_t));
    node_t *first;
    node_t *last;  // We use a zero node to mark the end of the list,
                   // this node cannot be modifyed, or access.

    assert( node_size_min >= sizeof(node_t) );

    size = size / node_alignsz * node_alignsz;  // Trim buffer size with alignment
    if( !buffer || size < 2*node_size_min ) return NULL;

    first = (node_t*)( (byte_t*)buffer + 0 );
    last  = (node_t*)( (byte_t*)buffer + size - node_size_min );
    memset(first, 0, sizeof(node_t));
    memset(last , 0, sizeof(node_t));
    first->size = size - node_size_min;

    return first;
}
//------------------------------------------------------------------------------
static
node_t* nodelist_find_freespace(node_t *start, size_t _reqsz)
{
    /*
     * 尋找串列中具有要求的緩衝空間大小，並且尚未被分配出去的節點。
     * @param start 傳入串列的起始結點，搜尋行為會從該結點開始。
     * @param reqsz 傳入要求的結點大小，函式會搜尋具有能夠滿足該空間需求的結點。
     *              注意這個數值指的是整個結點的大小，而不是結點緩衝空間的大小。
     * @return 搜尋成功時傳回結點指標；失敗時傳回 NULL。
     */
    size_t  reqsz = calc_align_size(_reqsz);
    node_t *node  = start;

    assert( reqsz >= _reqsz );

    while( node )
    {
        if( !node->inuse && node->size >= reqsz )
            break;

        node = node_get_next(node);
    }

    return node;
}
//------------------------------------------------------------------------------
//---- Memory Pool -------------------------------------------------------------
//------------------------------------------------------------------------------
mempool_t* mempool_init(void* buffer, size_t size)
{
    /**
     * @memberof mempool_t
     * @static
     * @brief Constructor.
     *
     * @param buffer 傳入一個供應給 Memory Pool 使用的資料空間。
     * @param size   傳入 buffer 的大小。
     * @return 成功時傳回初始化完成的 Memory Pool 指標，該位址與傳入的 buffer 位址相同，
     *         即除了表面型態的不同外，buffer 與 Memory Pool 是同一個物件。
     *         當 buffer 的空間不足以做為 Memory Pool 使用時會傳回 NULL。
     */
    mempool_t *pool = (mempool_t*) buffer;

    if( !buffer || size < sizeof(mempool_header_t) ) return NULL;

    if( !nodelist_init( pool->root, size - sizeof(mempool_header_t) ) ) return NULL;
    memset(pool, 0, sizeof(mempool_header_t));
    pool->size = size;

    return pool;
}
//------------------------------------------------------------------------------
uint64_t mempool_offset_from_addr(const mempool_t* pool, const void* bufaddr)
{
    /**
     * @memberof mempool_t
     * @brief   計算 Memory Pool 所分配緩衝空間在 Memory Pool 內的偏移量。
     * @details 本函式的目的為藉由統一的偏移量來標示資料位置，以便利不同地址空間的資料交換。
     *
     * @param pool    Memory Pool 物件。
     * @param bufaddr 傳入欲做轉換計算的緩衝空間起始位址，
     *                該位址必須為由本 Memory Pool 所分配傳回的位址。
     * @return 傳回 bufaddr 在 Memory Pool 內的偏移量。
     */
    uintptr_t buffpos = (uintptr_t) bufaddr;
    uintptr_t poolpos = (uintptr_t) pool;

    assert( pool );

    return ( poolpos < buffpos )?( buffpos - poolpos ):( 0 );
}
//------------------------------------------------------------------------------
void* mempool_offset_to_addr(const mempool_t* pool, uint64_t bufoff)
{
    /**
     * @memberof mempool_t
     * @brief   由緩衝區偏移量計算取得緩衝空間位址。
     * @details 本函式的目的為藉由統一的偏移量來標示資料位置，以便利不同地址空間的資料交換。
     *
     * @param pool   Memory Pool 物件。
     * @param bufoff 傳入一個以偏移量所標示的緩衝空間，
     *               該緩衝空間必須為由本 Memory Pool 所分配的空間。
     * @return 傳回由 bufoff 標示的緩衝空間起始位址。
     */
    uintptr_t poolpos = (uintptr_t) pool;

    assert( pool );

    return (void*)(( bufoff )?( poolpos + (uintptr_t)bufoff ):( 0 ));
}
//------------------------------------------------------------------------------
size_t mempool_get_pool_size(const mempool_t* pool)
{
    /**
     * @memberof mempool_t
     * @brief 查詢整個 Memory Pool 資料空間的大小。
     *
     * @param pool Memory Pool 物件。
     * @return 傳回 Memory Pool 所佔用資料空間的大小。
     */
    assert( pool );

    return pool->size;
}
//------------------------------------------------------------------------------
size_t mempool_get_free_size(const mempool_t* pool)
{
    /**
     * @memberof mempool_t
     * @brief 查詢 Memory Pool 中尚未被分配的緩衝空間總和。
     *
     * @param pool Memory Pool 物件。
     * @return 傳回 Memory Pool 中尚未被分配的緩衝空間總和。
     *         注意這個數值僅供參考，因為實際分配的緩衝空間還需要額外的資料練結資料空間。
     */
    const node_t *node;
    size_t        size = 0;

    assert( pool );

    node = pool->root;
    while( node )
    {
        if( !node->inuse )
            size += node->size - sizeof(node_t);

        node = node_get_next((node_t*)node);
    }

    return size;
}
//------------------------------------------------------------------------------
size_t mempool_get_buffer_size(const mempool_t* pool, const void* bufaddr)
{
    /**
     * @memberof mempool_t
     * @brief 查詢緩衝區空間大小。
     *
     * @param pool    Memory Pool 物件。
     * @param bufaddr 傳入欲做查詢的緩衝空間起始位址，
     *                該位址必須為由本 Memory Pool 所分配傳回的位址。
     * @return 傳回所查詢緩衝空間的大小。
     */
    const node_t *node;

    assert( pool );

    node = node_from_useraddr((byte_t*)bufaddr);
    return ( node )?( node->usersize ):( 0 );
}
//------------------------------------------------------------------------------
void* mempool_get_inuse_next(const mempool_t* pool, const void* bufaddr)
{
    /**
     * @memberof mempool_t
     * @brief   查詢下一個已分配的緩衝區。
     * @details 以一個 Memory Pool 所分配的緩衝區來查詢在該管理容器中的下一個已分配的緩衝區。
     *
     * @param pool    Memory Pool 物件。
     * @param bufaddr 傳入一個由本 Memory Pool 所分配的緩衝空間位址，此參數可為 NULL。
     * @return 在 bufaddr 有值時傳回 bufaddr 在本管理容器中的下一個已分配的緩衝空間位址；
     *         若 bufaddr 已為管理容器中的最後一個緩衝空間則傳回 NULL。
     *         在 bufaddr 為 NULL 時傳回該管理容器中的第一個以分配緩衝空間位址；
     *         若該管理容器內無任何已分配的緩衝區則傳回 NULL。
     */
    node_t *node = node_from_useraddr((byte_t*)bufaddr);

    assert( pool );

    node = ( node )?( node_get_next(node) ):( (node_t*)pool->root );
    while( node )
    {
        if( node->inuse ) break;
        node = node_get_next(node);
    }

    return ( node )?( node->userdata ):( NULL );
}
//------------------------------------------------------------------------------
void* mempool_allocate(mempool_t* pool, size_t size)
{
    /**
     * @memberof mempool_t
     * @brief 在 Memory Pool 中分配一個緩衝區空間。
     *
     * @param pool Memory Pool 物件。
     * @param size 指定新分配緩衝空間的大小，這個值就算是零，Memory Pool 也會嘗試分配一個緩衝位址。
     * @return 成功時傳回新分配緩衝空間的起始位址；失敗時傳回 NULL。
     */
    size_t  nodesize = sizeof(node_t) + size;
    node_t *node;

    assert( pool );

    if(( node = nodelist_find_freespace( pool->root, nodesize ) ))
    {
        node_separate(node, nodesize);
        node->usersize = size;
        node->inuse    = true;
    }

    return ( node )?( node->userdata ):( NULL );
}
//------------------------------------------------------------------------------
void mempool_deallocate(mempool_t* pool, void* bufaddr)
{
    /**
     * @memberof mempool_t
     * @brief 向 Memory Pool 歸還其所分配的緩衝空間。
     *
     * @param pool    Memory Pool 物件。
     * @param bufaddr 傳入欲歸還的緩衝空間起始位址，
     *                這個位址必須是由本 Memory Pool 所分配傳回的位址。
     */
    node_t *node_left;
    node_t *node_centre;
    node_t *node_right;

    assert( pool );

    node_centre = node_from_useraddr(bufaddr);
    if( !node_centre ) return;

    node_centre->inuse = false;

    node_left  = node_get_prev(node_centre);
    node_right = node_get_next(node_centre);

    if( node_right && !node_right->inuse ) node_merge_next(node_centre);
    if( node_left  && !node_left ->inuse ) node_merge_next(node_left  );
}
//------------------------------------------------------------------------------
void* mempool_reallocate(mempool_t* pool, void* bufaddr, size_t size)
{
    /**
     * @memberof mempool_t
     * @brief   變更緩衝區大小。
     * @details 更改一個已由 Memory Pool 所分配的緩衝空間大小。
     *
     * @param pool    Memory Pool 物件。
     * @param bufaddr 欲更改大小的緩衝空間起始位址，該位址必須為由本 Memory Pool 所分配傳回的位址。
     * @param size    指定新的緩衝空間大小，這個值就算是零，Memory Pool 也會嘗試分配一個緩衝位址。
     * @return 成功時傳回新分配的緩衝空間起始位址；失敗時傳回 NULL。
     *
     * @remarks
     *     @li 這個函式會首先嘗試在原來的位址上擴充緩衝空間，當目前的緩衝位址後無足夠的連續可用空間時，
     *         函式才會另闢新的緩衝空，並在搬移資料後釋放原來的空間。
     *         因此使用者必須注意，本函式所傳回的緩衝位址與使用者所傳入之位址不一定相同。
     *     @li 在重新分配緩衝空間時，函式會保存原有的資料。
     *         當新的緩衝空間比原來小時，空間後端的資料將被捨棄；而當新的空間比原來大時，
     *         新增的後部空間之內容不會被初始化設定，使用者必須視己所需對其進行設定。
     *     @li 當函式失敗時，原傳入之緩衝空間不會受到任何更動。
     */
    size_t  nodesz_required = sizeof(node_t) + size;
    node_t *node            = node_from_useraddr(bufaddr);

    assert( pool );

    if( !node || !node->inuse ) return NULL;

    // Change the user marked size directly if the current node have enough space.
    if( nodesz_required < node->size )
    {
        node->usersize = size;
        return node->userdata;
    }

    // Merge next node if it have enough space and is not in using.
    {
        node_t *node_next = node_get_next(node);

        if(  node_next                                      &&
            !node_next->inuse                               &&
             nodesz_required < node->size + node_next->size )
        {
            node_merge_next(node);
            assert( nodesz_required < node->size );

            node->usersize = size;
            return node->userdata;
        }
    }

    // Find another space
    {
        byte_t *newbuf = mempool_allocate(pool, size);
        if( !newbuf ) return NULL;

        memcpy(newbuf, node->userdata, node->usersize);
        mempool_deallocate(pool, node->userdata);

        return newbuf;
    }
}
//------------------------------------------------------------------------------

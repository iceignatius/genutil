#include <assert.h>
#include <stdlib.h>
#include "../minmax.h"
#include "vector.h"

//------------------------------------------------------------------------------
static
void* c99realloc(void* ptr, size_t size)
{
    if( size )
    {
        ptr = realloc(ptr, size);
    }
    else
    {
        free(ptr);
        ptr = NULL;
    }

    return ptr;
}
//------------------------------------------------------------------------------
static
void gvector_itemfree_default(void *item)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
void gvector_init(gvector_t *vector, gvector_itemfree_t itemfree)
{
    /**
     * @memberof gvector_t
     * @brief Constructor.
     *
     * @param vector    Object instance.
     * @param itemfree The function used to release an item.
     *                 This parameter can be NULL if not needed.
     */
    assert( vector );

    vector->items    = NULL;
    vector->count    = 0;
    vector->itemfree = itemfree ? itemfree : gvector_itemfree_default;
}
//------------------------------------------------------------------------------
void gvector_init_movefrom(gvector_t *vector, gvector_t *src)
{
    /**
     * @memberof gvector_t
     * @brief Construct, and move data from another container.
     *
     * @param vector Object instance.
     * @param src   Another container object to move data from.
     */
    assert( vector && src );

    gvector_init(vector, NULL);
    gvector_movefrom(vector, src);
}
//------------------------------------------------------------------------------
void gvector_deinit(gvector_t *vector)
{
    /**
     * @memberof gvector_t
     * @brief Destructor.
     *
     * @param vector Object instance.
     */
    assert( vector );
    gvector_clear(vector);
}
//------------------------------------------------------------------------------
void* gvector_get_item(gvector_t *vector, size_t index)
{
    /**
     * @memberof gvector_t
     * @brief Get item.
     *
     * @param vector Object instance.
     * @param index Position of the item.
     * @return Value of the item stored in container;
     *         or NULL if failed.
     */
    assert( vector );
    return index < vector->count ? vector->items[index] : NULL;
}
//------------------------------------------------------------------------------
const void* gvector_get_citem(const gvector_t *vector, size_t index)
{
    /**
     * @memberof gvector_t
     * @brief Get item.
     *
     * @param vector Object instance.
     * @param index Position of the item.
     * @return Value of the item stored in container;
     *         or NULL if failed.
     */
    assert( vector );
    return gvector_get_item((gvector_t*)vector, index);
}
//------------------------------------------------------------------------------
void gvector_set_item(gvector_t *vector, size_t index, void *item)
{
    /**
     * @memberof gvector_t
     * @brief Set item.
     *
     * @param vector Object instance.
     * @param index The position to set or update item.
     * @param item  The item to be added to the container.
     *
     * @remarks This function will add the item to the specific position,
     *          and the old item on the position will be released.
     *          Or the new item will be add at the end of container as push back function
     *          if the index is not and valid position.
     */
    assert( vector );

    if( index < vector->count )
    {
        vector->itemfree(vector->items[index]);
        vector->items[index] = item;
    }
    else
    {
        gvector_push_back(vector, item);
    }
}
//------------------------------------------------------------------------------
void gvector_push_front(gvector_t *vector, void *item)
{
    /**
     * @memberof gvector_t
     * @brief Push an item to the front of container.
     *
     * @param vector Object instance.
     * @param item  The item to be added to the container.
     */
    assert( vector );
    gvector_insert(vector, 0, item);
}
//------------------------------------------------------------------------------
void gvector_pop_front(gvector_t *vector)
{
    /**
     * @memberof gvector_t
     * @brief Pop an item from the front of container.
     *
     * @param vector Object instance.
     */
    assert( vector );
    gvector_erase(vector, 0);
}
//------------------------------------------------------------------------------
void gvector_push_back(gvector_t *vector, void *item)
{
    /**
     * @memberof gvector_t
     * @brief Push an item to the back of container.
     *
     * @param vector Object instance.
     * @param item  The item to be added to the container.
     */
    assert( vector );
    gvector_insert(vector, -1, item);
}
//------------------------------------------------------------------------------
void gvector_pop_back(gvector_t *vector)
{
    /**
     * @memberof gvector_t
     * @brief Pop an item from the back of container.
     *
     * @param vector Object instance.
     */
    assert( vector );
    gvector_erase(vector, -1);
}
//------------------------------------------------------------------------------
void gvector_insert(gvector_t *vector, size_t index, void *item)
{
    /**
     * @memberof gvector_t
     * @brief Insert an item to a specific position.
     *
     * @param vector Object instance.
     * @param index The position to insert item.
     *        And note that, if the index great then the highest valid index,
     *        the index will be treated to the last insert position,
     *        that means the function will be treated as push back function.
     * @param item  The item to be added to the container.
     */
    assert( vector );

    index = MIN( index, vector->count );

    size_t newsize = ( vector->count + 1 )*sizeof(vector->items[0]);
    vector->items = c99realloc(vector->items, newsize);
    assert( vector->items );

    size_t i;
    for(i=vector->count; i>index; --i)
        vector->items[i] = vector->items[i-1];
    vector->items[index] = item;

    ++ vector->count;
}
//------------------------------------------------------------------------------
void gvector_erase(gvector_t *vector, size_t index)
{
    /**
     * @memberof gvector_t
     * @brief Erase an item by a specific position.
     *
     * @param vector Object instance.
     *        And note that, if the index great then the highest valid index,
     *        the index will be treated to the last valid position,
     *        that means the function will be treated as push back function.
     * @param index Position of the item to be erased.
     */
    assert( vector );

    if( !vector->count ) return;
    index = MIN( index, vector->count - 1 );

    vector->itemfree(vector->items[index]);

    size_t i;
    for(i=index+1; i<vector->count; ++i)
        vector->items[i-1] = vector->items[i];

    size_t newsize = ( vector->count - 1 )*sizeof(vector->items[0]);
    vector->items = c99realloc(vector->items, newsize);

    -- vector->count;
}
//------------------------------------------------------------------------------
void gvector_clear(gvector_t *vector)
{
    /**
     * @memberof gvector_t
     * @brief Erase all items.
     *
     * @param vector Object instance.
     */
    assert( vector );

    size_t i;
    for(i=0; i<vector->count; ++i)
        vector->itemfree(vector->items[i]);

    vector->items = c99realloc(vector->items, 0);
    vector->count = 0;
}
//------------------------------------------------------------------------------
void gvector_movefrom(gvector_t *vector, gvector_t *src)
{
    /**
     * @memberof gvector_t
     * @brief Move and import items from another container.
     *
     * @param vector Object instance.
     * @param src   The data source.
     *
     * @remarks All old items stored in this container will be erased.
     */
    assert( vector && src );

    gvector_clear(vector);
    *vector = *src;

    src->items = NULL;
    src->count = 0;
}
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdlib.h>
#include "list.h"

//------------------------------------------------------------------------------
//---- List Node ---------------------------------------------------------------
//------------------------------------------------------------------------------
typedef glist_node_t node_t;
//------------------------------------------------------------------------------
static
node_t* node_create(void *value)
{
    node_t *node = malloc(sizeof(node_t));
    assert( node );

    node->prev  = NULL;
    node->next  = NULL;
    node->value = value;

    return node;
}
//------------------------------------------------------------------------------
static
void node_release(node_t *node)
{
    free(node);
}
//------------------------------------------------------------------------------
static
void node_link_head(node_t *main, node_t *newone)
{
    if( !main ) return;

    node_t *front = main->prev;
    if( front  ) front ->next = newone;
    if( newone )
    {
        newone->prev = front;
        newone->next = main;
    }
    main->prev = newone;
}
//------------------------------------------------------------------------------
static
void node_link_rear(node_t *main, node_t *newone)
{
    if( !main ) return;

    node_t *after = main->next;
    if( after ) after->prev = newone;
    if( newone )
    {
        newone->prev = main;
        newone->next = after;
    }
    main->next = newone;
}
//------------------------------------------------------------------------------
static
void node_unlink(node_t *node)
{
    if( !node ) return;

    node_t *prev = node->prev;
    node_t *next = node->next;
    if( prev ) prev->next = next;
    if( next ) next->prev = prev;
}
//------------------------------------------------------------------------------
INLINE
node_t* node_get_prev(node_t *node)
{
    return node ? node->prev : NULL;
}
//------------------------------------------------------------------------------
INLINE
node_t* node_get_next(node_t *node)
{
    return node ? node->next : NULL;
}
//------------------------------------------------------------------------------
INLINE
void* node_get_value(node_t *node)
{
    return node ? node->value : NULL;
}
//------------------------------------------------------------------------------
//---- Iterator ----------------------------------------------------------------
//------------------------------------------------------------------------------
static
void glist_iter_init(glist_iter_t *iter, const glist_t *list, node_t *node)
{
    assert( iter );

    iter->container = list;
    iter->node      = node;
}
//------------------------------------------------------------------------------
bool glist_iter_is_available(const glist_iter_t *iter)
{
    /**
     * @memberof glist_iter_t
     * @brief Check if the iterator is available.
     *
     * @param iter Object instance.
     * @return TRUE if it is available; and FALSE if not.
     */
    assert( iter );
    return iter->node;
}
//------------------------------------------------------------------------------
bool glist_iter_move_prev(glist_iter_t *iter)
{
    /**
     * @memberof glist_iter_t
     * @brief Move iterator to the previous position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = ( iter->node ? iter->node->prev : NULL ) );
}
//------------------------------------------------------------------------------
bool glist_iter_move_next(glist_iter_t *iter)
{
    /**
     * @memberof glist_iter_t
     * @brief Move iterator to the next position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = ( iter->node ? iter->node->next : NULL ) );
}
//------------------------------------------------------------------------------
void* glist_iter_get_value(glist_iter_t *iter)
{
    /**
     * @memberof glist_iter_t
     * @brief Get the item located by this iterator.
     *
     * @param iter Object instance.
     * @return The item located by this iterator,
     *         or NULL if there have no item.
     */
    assert( iter );
    return iter->node ? iter->node->value : NULL;
}
//------------------------------------------------------------------------------
bool glist_iter_set_value(glist_iter_t *iter, void *item)
{
    /**
     * @memberof glist_iter_t
     * @brief Set the item located by this iterator.
     *
     * @param iter Object instance.
     * @param item An item to replace the old one.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );

    if( !iter->node ) return false;

    const glist_t *list = iter->container;
    list->itemfree(iter->node->value);
    iter->node->value = item;

    return true;
}
//------------------------------------------------------------------------------
//---- Constant Iterator -------------------------------------------------------
//------------------------------------------------------------------------------
static
void glist_citer_init(glist_citer_t *iter, const glist_t *list, const node_t *node)
{
    assert( iter );

    iter->container = list;
    iter->node      = node;
}
//------------------------------------------------------------------------------
bool glist_citer_is_available(const glist_citer_t *iter)
{
    /**
     * @memberof glist_citer_t
     * @brief Check if the iterator is available.
     *
     * @param iter Object instance.
     * @return TRUE if it is available; and FALSE if not.
     */
    assert( iter );
    return iter->node;
}
//------------------------------------------------------------------------------
bool glist_citer_move_prev(glist_citer_t *iter)
{
    /**
     * @memberof glist_citer_t
     * @brief Move iterator to the previous position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = ( iter->node ? iter->node->prev : NULL ) );
}
//------------------------------------------------------------------------------
bool glist_citer_move_next(glist_citer_t *iter)
{
    /**
     * @memberof glist_citer_t
     * @brief Move iterator to the next position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = ( iter->node ? iter->node->next : NULL ) );
}
//------------------------------------------------------------------------------
const void* glist_citer_get_value(const glist_citer_t *iter)
{
    /**
     * @memberof glist_citer_t
     * @brief Get the item located by this iterator.
     *
     * @param iter Object instance.
     * @return The item located by this iterator,
     *         or NULL if there have no item.
     */
    assert( iter );
    return iter->node ? iter->node->value : NULL;
}
//------------------------------------------------------------------------------
//---- List Class --------------------------------------------------------------
//------------------------------------------------------------------------------
static
void glist_itemfree_default(void *item)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
void glist_init(glist_t *list, glist_itemfree_t itemfree)
{
    /**
     * @memberof glist_t
     * @brief Constructor.
     *
     * @param list     Object instance.
     * @param itemfree The function used to release an item.
     *                 This parameter can be NULL if not needed.
     */
    assert( list );

    list->first    = NULL;
    list->last     = NULL;
    list->count    = 0;
    list->itemfree = itemfree ? itemfree : glist_itemfree_default;
}
//------------------------------------------------------------------------------
void glist_init_movefrom(glist_t *list, glist_t *src)
{
    /**
     * @memberof glist_t
     * @brief Construct, and move data from another container.
     *
     * @param list Object instance.
     * @param src  Another container object to move data from.
     */
    assert( list && src );

    glist_init(list, NULL);
    glist_movefrom(list, src);
}
//------------------------------------------------------------------------------
void glist_deinit(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Destructor.
     *
     * @param list Object instance.
     */
    assert( list );

    glist_clear(list);
}
//------------------------------------------------------------------------------
glist_iter_t glist_get_first(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Get the first item.
     *
     * @param list Object instance.
     * @return An iterator of the first item,
     *         and that may be an invalid iterator if there have no any item.
     */
    assert( list );

    glist_iter_t iter;
    glist_iter_init(&iter, list, list->first);

    return iter;
}
//------------------------------------------------------------------------------
glist_iter_t glist_get_last(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Get the last item.
     *
     * @param list Object instance.
     * @return An iterator of the last item,
     *         and that may be an invalid iterator if there have no any item.
     */
    assert( list );

    glist_iter_t iter;
    glist_iter_init(&iter, list, list->last);

    return iter;
}
//------------------------------------------------------------------------------
glist_citer_t glist_get_cfirst(const glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Get the first item.
     *
     * @param list Object instance.
     * @return A constant iterator of the first item,
     *         and that may be an invalid iterator if there have no any item.
     */
    assert( list );

    glist_citer_t iter;
    glist_citer_init(&iter, list, list->first);

    return iter;
}
//------------------------------------------------------------------------------
glist_citer_t glist_get_clast(const glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Get the last item.
     *
     * @param list Object instance.
     * @return A constant iterator of the last item,
     *         and that may be an invalid iterator if there have no any item.
     */
    assert( list );

    glist_citer_t iter;
    glist_citer_init(&iter, list, list->last);

    return iter;
}
//------------------------------------------------------------------------------
void glist_push_front(glist_t *list, void *item)
{
    /**
     * @memberof glist_t
     * @brief Push an item to the front of container.
     *
     * @param list Object instance.
     * @param item The item to be added to the container.
     */
    assert( list );

    node_t *node = node_create(item);
    node_link_head(list->first, node);
    list->first = node;
    if( !list->last ) list->last = node;

    ++list->count;
}
//------------------------------------------------------------------------------
void glist_pop_front(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Pop an item from the front of container.
     *
     * @param list Object instance.
     */
    assert( list );

    node_t *node = list->first;
    if( !node ) return;

    list->first = node_get_next(node);
    if( !list->first ) list->last = NULL;
    node_unlink(node);

    list->itemfree(node_get_value(node));
    node_release(node);

    --list->count;
}
//------------------------------------------------------------------------------
void glist_push_back(glist_t *list, void *item)
{
    /**
     * @memberof glist_t
     * @brief Push an item to the back of container.
     *
     * @param list Object instance.
     * @param item The item to be added to the container.
     */
    assert( list );

    node_t *node = node_create(item);
    node_link_rear(list->last, node);
    list->last = node;
    if( !list->first ) list->first = node;

    ++list->count;
}
//------------------------------------------------------------------------------
void glist_pop_back(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Pop an item from the back of container.
     *
     * @param list Object instance.
     */
    assert( list );

    node_t *node = list->last;
    if( !node ) return;

    list->last = node_get_prev(node);
    if( !list->last ) list->first = NULL;
    node_unlink(node);

    list->itemfree(node_get_value(node));
    node_release(node);

    --list->count;
}
//------------------------------------------------------------------------------
void glist_insert(glist_t *list, glist_iter_t *pos, void *item)
{
    /**
     * @memberof glist_t
     * @brief Insert an item to a specific position.
     *
     * @param list Object instance.
     * @param pos  The position to insert item.
     * @param item The item to be added to the container.
     */
    assert( list && pos );
    assert( pos->container == list );

    node_t *node = node_create(item);
    node_link_head(pos->node, node);
    if( list->first == pos->node ) list->first = node;
    if( !list->last ) list->last = node;

    ++list->count;
}
//------------------------------------------------------------------------------
void glist_erase(glist_t *list, glist_iter_t *pos)
{
    /**
     * @memberof glist_t
     * @brief Erase an item by a specific position.
     *
     * @param list Object instance.
     * @param pos  Position of the item to be erased.
     */
    assert( list && pos );
    assert( pos->container == list );

    node_t *node = pos->node;
    if( !node ) return;

    if( list->first == node ) list->first = node_get_next(node);
    if( list->last  == node ) list->last  = node_get_prev(node);
    node_unlink(node);

    list->itemfree(node_get_value(node));
    node_release(node);

    --list->count;
}
//------------------------------------------------------------------------------
void glist_clear(glist_t *list)
{
    /**
     * @memberof glist_t
     * @brief Erase all items.
     *
     * @param list Object instance.
     */
    assert( list );

    node_t *node = list->first;
    while( node )
    {
        node_t *temp = node;
        node = node_get_next(node);

        list->itemfree(node_get_value(temp));
        node_release(temp);
    }

    list->first = NULL;
    list->last  = NULL;
    list->count = 0;
}
//------------------------------------------------------------------------------
void glist_movefrom(glist_t *list, glist_t *src)
{
    /**
     * @memberof glist_t
     * @brief Move and import items from another container.
     *
     * @param list Object instance.
     * @param src  The data source.
     *
     * @remarks All old items stored in this container will be erased.
     */
    assert( list && src );

    if( list == src ) return;

    glist_clear(list);
    list->itemfree = src->itemfree;

    list->first = src->first;
    list->last  = src->last;
    list->count = src->count;

    src->first = NULL;
    src->last  = NULL;
    src->count = 0;
}
//------------------------------------------------------------------------------

#include <assert.h>
#include <stdlib.h>
#include "queue.h"

//------------------------------------------------------------------------------
//---- Queue Node --------------------------------------------------------------
//------------------------------------------------------------------------------
typedef gqueue_node_t node_t;
//------------------------------------------------------------------------------
static
node_t* node_create(bool have_value, void *value)
{
    node_t *node = malloc(sizeof(node_t));
    assert( node );

    node->next       = NULL;
    node->have_value = have_value;
    node->value      = value;

    return node;
}
//------------------------------------------------------------------------------
static
void node_release(node_t *node, gqueue_itemfree_t itemfree)
{
    assert( node && itemfree );

    if( node->have_value )
        itemfree(node->value);

    free(node);
}
//------------------------------------------------------------------------------
static
void node_free_item_only(node_t *node, gqueue_itemfree_t itemfree)
{
    assert( node && itemfree );

    if( !node->have_value ) return;

    itemfree(node->value);
    node->have_value = false;
}
//------------------------------------------------------------------------------
//---- Queue Class -------------------------------------------------------------
//------------------------------------------------------------------------------
static
void gqueue_itemfree_default(void *item)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
static
void pop_one_node(gqueue_t *queue)
{
    assert( queue && queue->first );

    node_t *node = queue->first;
    queue->first = node->next;

    node_release(node, queue->itemfree);
}
//------------------------------------------------------------------------------
void gqueue_init(gqueue_t *queue, gqueue_itemfree_t itemfree)
{
    /**
     * @memberof gqueue_t
     * @brief Initialize a queue object
     *
     * @param queue    Object instance.
     * @param itemfree The function used to release an item.
     *                 This parameter can be NULL if not needed.
     */
    assert( queue );

    node_t *node_empty = node_create(false, NULL);

    queue->first    = node_empty;
    queue->last     = node_empty;
    queue->pushcnt  = 0;
    queue->popcnt   = 0;
    queue->itemfree = itemfree ? itemfree : gqueue_itemfree_default;
}
//------------------------------------------------------------------------------
void gqueue_deinit(gqueue_t *queue)
{
    /**
     * @memberof gqueue_t
     * @brief Destructor.
     *
     * @param queue Object instance.
     */
    assert( queue );

    while( queue->first )
        pop_one_node(queue);
}
//------------------------------------------------------------------------------
void* gqueue_get_first(gqueue_t *queue)
{
    /**
     * @memberof gqueue_t
     * @brief Get the first item.
     *
     * @param queue Object instance.
     * @return The first item of container;
               or NULL if there have no any item.
     */
    assert( queue );

    node_t *node = queue->first->have_value ? queue->first : queue->first->next;
    return ( node && node->have_value )?( node->value ):( NULL );
}
//------------------------------------------------------------------------------
void* gqueue_get_last(gqueue_t *queue)
{
    /**
     * @memberof gqueue_t
     * @brief Get the last item.
     *
     * @param queue Object instance.
     * @return The last item of container;
               or NULL if there have no any item.
     */
    assert( queue );

    node_t *node = queue->last;
    return ( node->have_value )?( node->value ):( NULL );
}
//------------------------------------------------------------------------------
void gqueue_clear(gqueue_t *queue)
{
    /**
     * @memberof gqueue_t
     * @brief Pop all items.
     *
     * @param queue Object instance.
     */
    assert( queue );

    while( gqueue_get_count(queue) )
        gqueue_pop(queue);
}
//------------------------------------------------------------------------------
void gqueue_push(gqueue_t *queue, void *item)
{
    /**
     * @memberof gqueue_t
     * @brief Push an item to the container.
     *
     * @param queue Object instance.
     * @param item  The item to be added to the container.
     */
    assert( queue );

    node_t *node = node_create(true, item);
    queue->last->next = node;
    queue->last       = node;

    ++ queue->pushcnt;
}
//------------------------------------------------------------------------------
void gqueue_pop(gqueue_t *queue)
{
    /**
     * @memberof gqueue_t
     * @brief Pop an item from the container.
     *
     * @param queue Object instance.
     */
    assert( queue );

    if( !queue->first->have_value )
    {
        if( !queue->first->next )
            return;
        else
            pop_one_node(queue);
    }

    node_t *node = queue->first;
    assert( node && node->have_value );

    if( node->next )
    {
        pop_one_node(queue);
    }
    else
    {
        node_free_item_only(node, queue->itemfree);
    }

    ++ queue->popcnt;
}
//------------------------------------------------------------------------------

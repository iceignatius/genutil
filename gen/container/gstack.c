#include <assert.h>
#include <stdlib.h>
#include "stack.h"

//------------------------------------------------------------------------------
//---- Stack Node --------------------------------------------------------------
//------------------------------------------------------------------------------
typedef gstack_node_t node_t;
//------------------------------------------------------------------------------
static
node_t* node_create(node_t *prev, void *value)
{
    node_t *node = malloc(sizeof(node_t));
    assert( node );

    node->prev  = prev;
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
//---- Stack Class -------------------------------------------------------------
//------------------------------------------------------------------------------
static
void gstack_itemfree_default(void *item)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
void gstack_init(gstack_t *stack, gstack_itemfree_t itemfree)
{
    /**
     * @memberof gstack_t
     * @brief Constructor.
     *
     * @param stack    Object instance.
     * @param itemfree The function used to release an item.
     *                 This parameter can be NULL if not needed.
     */
    assert( stack );

    stack->top      = NULL;
    stack->count    = 0;
    stack->itemfree = itemfree ? itemfree : gstack_itemfree_default;
}
//------------------------------------------------------------------------------
void gstack_init_movefrom(gstack_t *stack, gstack_t *src)
{
    /**
     * @memberof gstack_t
     * @brief Construct, and move data from another container.
     *
     * @param stack Object instance.
     * @param src   Another container object to move data from.
     */
    assert( stack && src );

    gstack_init(stack, NULL);
    gstack_movefrom(stack, src);
}
//------------------------------------------------------------------------------
void gstack_deinit(gstack_t *stack)
{
    /**
     * @memberof gstack_t
     * @brief Destructor.
     *
     * @param stack Object instance.
     */
    assert( stack );

    gstack_clear(stack);
}
//------------------------------------------------------------------------------
void gstack_push(gstack_t *stack, void *item)
{
    /**
     * @memberof gstack_t
     * @brief Push an item in to the top of container.
     *
     * @param stack Object instance.
     * @param item  The item to be added to the container.
     */
    assert( stack );

    node_t *node = node_create(stack->top, item);

    stack->top = node;
    ++ stack->count;
}
//------------------------------------------------------------------------------
void gstack_pop(gstack_t *stack)
{
    /**
     * @memberof gstack_t
     * @brief Pop the top item.
     *
     * @param stack Object instance.
     */
    assert( stack );

    node_t *node = stack->top;
    if( !node ) return;

    stack->top = node->prev;
    -- stack->count;

    stack->itemfree(node->value);
    node_release(node);
}
//------------------------------------------------------------------------------
void* gstack_get_top(gstack_t *stack)
{
    /**
     * @memberof gstack_t
     * @brief Get item at the top of container.
     *
     * @param stack Object instance.
     * @return The top item; or NULL if the container is empty.
     */
    assert( stack );

    return stack->top ? stack->top->value : NULL;
}
//------------------------------------------------------------------------------
void gstack_clear(gstack_t *stack)
{
    /**
     * @memberof gstack_t
     * @brief Erase all items.
     *
     * @param stack Object instance.
     */
    assert( stack );

    if( !stack->top ) return;

    node_t *node = stack->top;
    while( node )
    {
        node_t *nodedel = node;
        node = node->prev;

        stack->itemfree(nodedel->value);
        node_release(nodedel);
    }

    stack->top   = NULL;
    stack->count = 0;
}
//------------------------------------------------------------------------------
void gstack_movefrom(gstack_t *stack, gstack_t *src)
{
    /**
     * @memberof gstack_t
     * @brief Move and import items from another container.
     *
     * @param stack Object instance.
     * @param src   The data source.
     *
     * @remarks All old items stored in this container will be erased.
     */
    assert( stack && src );

    if( stack == src ) return;

    gstack_clear(stack);
    stack->itemfree = src->itemfree;

    stack->top   = src->top;
    stack->count = src->count;

    src->top   = NULL;
    src->count = 0;
}
//------------------------------------------------------------------------------

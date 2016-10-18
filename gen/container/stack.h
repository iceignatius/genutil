/**
 * @file
 * @brief     General container - Stack.
 * @details   To support a set of general container for C language.
 * @author    王文佑
 * @date      2015.12.21
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CONTAINER_STACK_H_
#define _GEN_CONTAINER_STACK_H_

#include <stddef.h>
#include <stdbool.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- Stack Node ----------------------
//--------------------------------------

typedef struct gstack_node_t
{
    // WARNING : All members are private!

    struct gstack_node_t *prev;

    void *value;

} gstack_node_t;

//--------------------------------------
//---- Callbacks -----------------------
//--------------------------------------

/**
 * @memberof gstack_t
 * @brief Callback when the container want to release an item.
 * @param item The item to be released.
 */
typedef void(*gstack_itemfree_t)(void *item);

//--------------------------------------
//---- Stack Class ---------------------
//--------------------------------------

/**
 * @class gstack_t
 * @brief Stack container.
 */
typedef struct gstack_t
{
    // WARNING : All members are private!

    gstack_node_t *top;
    size_t         count;

    gstack_itemfree_t itemfree;

} gstack_t;

// constructor and destructor
void gstack_init         (gstack_t *stack, gstack_itemfree_t itemfree);
void gstack_init_movefrom(gstack_t *stack, gstack_t *src);
void gstack_deinit       (gstack_t *stack);

// capacity
/// @memberof gstack_t @brief Get items count.
INLINE size_t gstack_get_count(const gstack_t *stack) { return stack ? stack->count : 0; }
/// @memberof gstack_t @brief Check if the container is empty.
INLINE bool   gstack_is_empty (const gstack_t *stack) { return !gstack_get_count(stack); }

// modifier and value getter
void  gstack_push   (gstack_t *stack, void *item);
void  gstack_pop    (gstack_t *stack);
void* gstack_get_top(gstack_t *stack);

/// @overload gstack_get_top
INLINE const void* gstack_get_ctop(const gstack_t *stack) { return gstack_get_top((gstack_t*)stack); }

// modifier for whole object
void gstack_clear   (gstack_t *stack);
void gstack_movefrom(gstack_t *stack, gstack_t *src);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

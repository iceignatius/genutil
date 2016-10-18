/**
 * @file
 * @brief     General container - List.
 * @details   To support a set of general container for C language.
 * @author    王文佑
 * @date      2015.01.26
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CONTAINER_LIST_H_
#define _GEN_CONTAINER_LIST_H_

#include <stddef.h>
#include <stdbool.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- List Node -----------------------
//--------------------------------------

typedef struct glist_node_t
{
    // WARNING : All members are private!

    struct glist_node_t *prev;
    struct glist_node_t *next;

    void *value;

} glist_node_t;

//--------------------------------------
//---- Iterator ------------------------
//--------------------------------------

/**
 * @class glist_iter_t
 * @brief Iterator of @ref glist_t.
 */
typedef struct glist_iter_t
{
    // WARNING : All members are private!
    const struct glist_t *container;
    glist_node_t         *node;
} glist_iter_t;

bool  glist_iter_is_available(const glist_iter_t *iter);
bool  glist_iter_move_prev   (      glist_iter_t *iter);
bool  glist_iter_move_next   (      glist_iter_t *iter);
void* glist_iter_get_value   (      glist_iter_t *iter);
bool  glist_iter_set_value   (      glist_iter_t *iter, void *item);

//--------------------------------------
//---- Constant Iterator ---------------
//--------------------------------------

/**
 * @class glist_citer_t
 * @brief Constant iterator of @ref glist_t.
 */
typedef struct glist_citer_t
{
    // WARNING : All members are private!
    const struct glist_t *container;
    const glist_node_t   *node;
} glist_citer_t;

bool        glist_citer_is_available(const glist_citer_t *iter);
bool        glist_citer_move_prev   (      glist_citer_t *iter);
bool        glist_citer_move_next   (      glist_citer_t *iter);
const void* glist_citer_get_value   (const glist_citer_t *iter);

//--------------------------------------
//---- Callbacks -----------------------
//--------------------------------------

/**
 * @memberof glist_t
 * @brief Callback when the container want to release an item.
 * @param item The item to be released.
 */
typedef void(*glist_itemfree_t)(void *item);

//--------------------------------------
//---- List Class ----------------------
//--------------------------------------

/**
 * @class glist_t
 * @brief List container.
 */
typedef struct glist_t
{
    // WARNING : All members are private!

    glist_node_t *first;
    glist_node_t *last;
    size_t        count;

    glist_itemfree_t itemfree;

} glist_t;

// constructor and destructor
void glist_init         (glist_t *list, glist_itemfree_t itemfree);
void glist_init_movefrom(glist_t *list, glist_t *src);
void glist_deinit       (glist_t *list);

// iterator
glist_iter_t  glist_get_first (      glist_t *list);
glist_iter_t  glist_get_last  (      glist_t *list);
glist_citer_t glist_get_cfirst(const glist_t *list);
glist_citer_t glist_get_clast (const glist_t *list);

// capacity
/// @memberof glist_t @brief Get items count.
INLINE size_t glist_get_count(const glist_t *list) { return list ? list->count : 0; }
/// @memberof glist_t @brief Check if the container is empty.
INLINE bool   glist_is_empty (const glist_t *list) { return !glist_get_count(list); }

// modifier for single item
void glist_push_front(glist_t *list, void *item);
void glist_pop_front (glist_t *list);
void glist_push_back (glist_t *list, void *item);
void glist_pop_back  (glist_t *list);
void glist_insert    (glist_t *list, glist_iter_t *pos, void *item);
void glist_erase     (glist_t *list, glist_iter_t *pos);

// modifier for whole object
void glist_clear   (glist_t *list);
void glist_movefrom(glist_t *list, glist_t *src);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

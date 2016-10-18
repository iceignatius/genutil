/**
 * @file
 * @brief     General container - Vector.
 * @details   To support a set of general container for C language.
 * @author    王文佑
 * @date      2015.08.21
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CONTAINER_VECTOR_H_
#define _GEN_CONTAINER_VECTOR_H_

#include <stddef.h>
#include <stdbool.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- Callbacks -----------------------
//--------------------------------------

/**
 * @memberof gvector_t
 * @brief Callback when the container want to release an item.
 * @param item The item to be released.
 */
typedef void(*gvector_itemfree_t)(void *item);

//--------------------------------------
//---- Vector Class --------------------
//--------------------------------------

/**
 * @class gvector_t
 * @brief Vector container.
 */
typedef struct gvector_t
{
    // WARNING : All members are private!

    void   **items;
    size_t   count;

    gvector_itemfree_t itemfree;

} gvector_t;

// constructor and destructor
void gvector_init         (gvector_t *vector, gvector_itemfree_t itemfree);
void gvector_init_movefrom(gvector_t *vector, gvector_t *src);
void gvector_deinit       (gvector_t *vector);

// item access
void*       gvector_get_item (      gvector_t *vector, size_t index);
const void* gvector_get_citem(const gvector_t *vector, size_t index);
void        gvector_set_item (      gvector_t *vector, size_t index, void *item);

// capacity
/// @memberof gvector_t @brief Get items count.
INLINE size_t gvector_get_count(const gvector_t *vector) { return vector ? vector->count : 0; }
/// @memberof gvector_t @brief Check if the container is empty.
INLINE bool   gvector_is_empty (const gvector_t *vector) { return !gvector_get_count(vector); }

// modifier for single item
void gvector_push_front(gvector_t *vector, void *item);
void gvector_pop_front (gvector_t *vector);
void gvector_push_back (gvector_t *vector, void *item);
void gvector_pop_back  (gvector_t *vector);
void gvector_insert    (gvector_t *vector, size_t index, void *item);
void gvector_erase     (gvector_t *vector, size_t index);

// modifier for whole object
void gvector_clear   (gvector_t *vector);
void gvector_movefrom(gvector_t *vector, gvector_t *src);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

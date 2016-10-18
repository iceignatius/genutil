/**
 * @file
 * @brief     General container - Tree Map.
 * @details   To support a set of general container for C language.
 * @author    王文佑
 * @date      2015.02.07
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CONTAINER_MAP_H_
#define _GEN_CONTAINER_MAP_H_

#include <stddef.h>
#include <stdbool.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- Map Node ------------------------
//--------------------------------------

typedef struct gmap_node_t
{
    // WARNING : All members are private!

    struct gmap_node_t *parent;
    struct gmap_node_t *left;
    struct gmap_node_t *right;

    bool red;

    void *tag;
    void *value;

    const struct gmap_events_t *events;

} gmap_node_t;


//--------------------------------------
//---- Iterator ------------------------
//--------------------------------------

/**
 * @class gmap_iter_t
 * @brief Iterator of @ref gmap_t.
 */
typedef struct gmap_iter_t
{
    // WARNING : All members are private!
    const struct gmap_t *container;
    gmap_node_t         *node;
} gmap_iter_t;

bool        gmap_iter_is_available(const gmap_iter_t *iter);
bool        gmap_iter_move_prev   (      gmap_iter_t *iter);
bool        gmap_iter_move_next   (      gmap_iter_t *iter);
const void* gmap_iter_get_tag     (const gmap_iter_t *iter);
void*       gmap_iter_get_value   (      gmap_iter_t *iter);
bool        gmap_iter_set_value   (      gmap_iter_t *iter, void *value);

//--------------------------------------
//---- Constant Iterator ---------------
//--------------------------------------

/**
 * @class gmap_citer_t
 * @brief Constant iterator of @ref gmap_t.
 */
typedef struct gmap_citer_t
{
    // WARNING : All members are private!
    const struct gmap_t *container;
    const gmap_node_t   *node;
} gmap_citer_t;

bool        gmap_citer_is_available(const gmap_citer_t *iter);
bool        gmap_citer_move_prev   (      gmap_citer_t *iter);
bool        gmap_citer_move_next   (      gmap_citer_t *iter);
const void* gmap_citer_get_tag     (const gmap_citer_t *iter);
const void* gmap_citer_get_value   (const gmap_citer_t *iter);

//--------------------------------------
//---- Callbacks -----------------------
//--------------------------------------

/**
 * @memberof gmap_t
 * @brief Callback when the container want to release an tag.
 * @param tag The tag to be released.
 */
typedef void(*gmap_tagfree_t)(void *tag);

/**
 * @memberof gmap_t
 * @brief Callback when the container want to release an item.
 * @param item The item to be released.
 */
typedef void(*gmap_itemfree_t)(void *item);

/**
 * @memberof gmap_t
 * @brief Callback to compare two tags.
 * @param tag1 Tag 1.
 * @param tag1 Tag 2.
 * @return
 *     @li A NEGATIVE value if @a tag1 less then @a tag2; and
 *     @li a POSITIVE value if @a tag1 great then @a tag2; and
 *     @li a ZERO value if the two are equal.
 */
typedef int(*gmap_tagcmp_t)(const void *tag1, const void *tag2);

//--------------------------------------
//---- Map Class -----------------------
//--------------------------------------

/**
 * @class gmap_t
 * @brief Map container.
 */
typedef struct gmap_t
{
    // WARNING : All members are private!

    gmap_node_t *root;
    size_t       count;

    struct gmap_events_t *events;

} gmap_t;

// constructor and destructor
void gmap_init         (gmap_t *map, gmap_tagcmp_t   tagcmp,
                                     gmap_tagfree_t  tagfree,
                                     gmap_itemfree_t itemfree);
void gmap_init_movefrom(gmap_t *map, gmap_t *src);
void gmap_deinit       (gmap_t *map);

// iterator
gmap_iter_t  gmap_get_first (      gmap_t *map);
gmap_iter_t  gmap_get_last  (      gmap_t *map);
gmap_citer_t gmap_get_cfirst(const gmap_t *map);
gmap_citer_t gmap_get_clast (const gmap_t *map);

// capacity
/// @memberof gmap_t @brief Get items count.
INLINE size_t gmap_get_count(const gmap_t *map) { return map->count; }
/// @memberof gmap_t @brief Check if the container is empty.
INLINE bool   gmap_is_empty (const gmap_t *map) { return !gmap_get_count(map); }

// Search
gmap_iter_t  gmap_find      (      gmap_t *map, const void *tag);
gmap_citer_t gmap_cfind     (const gmap_t *map, const void *tag);
void*        gmap_find_item (      gmap_t *map, const void *tag);
const void*  gmap_find_citem(const gmap_t *map, const void *tag);

// modifier for single item
void gmap_insert     (gmap_t *map, void *tag, void *value);
void gmap_erase      (gmap_t *map, gmap_iter_t *pos);
void gmap_erase_bytag(gmap_t *map, const void *tag);

// modifier for whole object
void gmap_clear   (gmap_t *map);
void gmap_movefrom(gmap_t *map, gmap_t *src);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

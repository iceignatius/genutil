/**
 * @file
 * @brief     General container - Queue.
 * @details   To support a set of general container for C language.
 * @author    王文佑
 * @date      2015.02.13
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_CONTAINER_QUEUE_H_
#define _GEN_CONTAINER_QUEUE_H_

#include <stddef.h>
#include <stdbool.h>
#include "../inline.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- Queue Node ----------------------
//--------------------------------------

typedef struct gqueue_node_t
{
    // WARNING : All members are private!

    struct gqueue_node_t *next;

    bool  have_value;
    void *value;

} gqueue_node_t;

//--------------------------------------
//---- Callbacks -----------------------
//--------------------------------------

/**
 * @memberof gqueue_t
 * @brief Callback when the container want to release an item.
 * @param item The item to be released.
 */
typedef void(*gqueue_itemfree_t)(void *item);

//--------------------------------------
//---- Queue Class ---------------------
//--------------------------------------

/**
 * @class gqueue_t
 * @brief   Queue container.
 * @details This container is desged for @b multi-threads application
 *          that data can be pushing and popping in different threads without lock,
 *          and all property/status query functions are thread safe, too.
 */
typedef struct gqueue_t
{
    // WARNING : All members are private!

    gqueue_node_t *first;
    gqueue_node_t *last;
    size_t         pushcnt;
    size_t         popcnt;

    gqueue_itemfree_t itemfree;

} gqueue_t;

// constructor and destructor
void gqueue_init  (gqueue_t *queue, gqueue_itemfree_t itemfree);
void gqueue_deinit(gqueue_t *queue);

// capacity
/// @memberof gqueue_t @brief Get items count.
INLINE size_t gqueue_get_count(const gqueue_t *queue) { return queue->pushcnt - queue->popcnt; }
/// @memberof gqueue_t @brief Check if the container is empty.
INLINE bool   gqueue_is_empty (const gqueue_t *queue) { return !gqueue_get_count(queue); }

// iterator
void* gqueue_get_first(gqueue_t *queue);
void* gqueue_get_last (gqueue_t *queue);

// Modifier
void gqueue_clear(gqueue_t *queue);
void gqueue_push (gqueue_t *queue, void *item);
void gqueue_pop  (gqueue_t *queue);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

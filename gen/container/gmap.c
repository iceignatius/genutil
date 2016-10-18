#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "map.h"

//------------------------------------------------------------------------------
//---- User Event Table --------------------------------------------------------
//------------------------------------------------------------------------------
typedef struct gmap_events_t
{
    gmap_tagcmp_t   tagcmp;
    gmap_tagfree_t  tagfree;
    gmap_itemfree_t itemfree;
} gmap_events_t;
//------------------------------------------------------------------------------
static
gmap_events_t* gmap_events_create(gmap_tagcmp_t   tagcmp,
                                  gmap_tagfree_t  tagfree,
                                  gmap_itemfree_t itemfree)
{
    gmap_events_t *tbl = malloc(sizeof(gmap_events_t));
    assert( tbl );

    tbl->tagcmp   = tagcmp;
    tbl->tagfree  = tagfree;
    tbl->itemfree = itemfree;

    return tbl;
}
//------------------------------------------------------------------------------
static
void gmap_events_release(gmap_events_t *tbl)
{
    assert( tbl );
    free(tbl);
}
//------------------------------------------------------------------------------
//---- Map Node ----------------------------------------------------------------
//------------------------------------------------------------------------------
typedef gmap_node_t node_t;
//------------------------------------------------------------------------------
//---- Map Node - Life and Payload ---------------------------------------------
//------------------------------------------------------------------------------
static
node_t* node_create(void *tag, void *value, const gmap_events_t *events)
{
    assert( events );

    node_t *node = malloc(sizeof(node_t));
    assert( node );

    node->parent = NULL;
    node->left   = NULL;
    node->right  = NULL;
    node->red    = true;
    node->tag    = tag;
    node->value  = value;
    node->events = events;

    return node;
}
//------------------------------------------------------------------------------
static
void node_release(node_t *node)
{
    assert( node );

    node->events->tagfree (node->tag);
    node->events->itemfree(node->value);

    free(node);
}
//------------------------------------------------------------------------------
static
void node_replace_tag_value(node_t *node, void *tag, void *value)
{
    assert( node );

    if( node->tag != tag )
    {
        node->events->tagfree(node->tag);
        node->tag = tag;
    }
    if( node->value != value )
    {
        node->events->itemfree(node->value);
        node->value = value;
    }
}
//------------------------------------------------------------------------------
//---- Map Node - Get Property -------------------------------------------------
//------------------------------------------------------------------------------
static
bool node_is_isolated(const node_t *node)
{
    assert( node );
    return !node->parent && !node->left && !node->right;
}
//------------------------------------------------------------------------------
static
bool node_is_black(node_t *node)
{
    return !node || !node->red;
}
//------------------------------------------------------------------------------
static
bool node_is_red(node_t *node)
{
    return node && node->red;
}
//------------------------------------------------------------------------------
static
bool node_have_child(node_t *node)
{
    return node && ( node->left || node->right );
}
//------------------------------------------------------------------------------
static
bool node_have_full_child(node_t *node)
{
    return node && node->left && node->right;
}
//------------------------------------------------------------------------------
//---- Map Node - Get Relative Node --------------------------------------------
//------------------------------------------------------------------------------
static
node_t* node_get_leftmost_child(node_t *node)
{
    while( node && node->left )
        node = node->left;

    return node;
}
//------------------------------------------------------------------------------
static
node_t* node_get_rightmost_child(node_t *node)
{
    while( node && node->right )
        node = node->right;

    return node;
}
//------------------------------------------------------------------------------
static
node_t* node_get_leftmost_parent(node_t *node)
{
    if( !node ) return NULL;

    node_t *parent = node->parent;
    while( parent && parent->right == node )
    {
        node   = parent;
        parent = node->parent;
    }

    return node;
}
//------------------------------------------------------------------------------
static
node_t* node_get_rightmost_parent(node_t *node)
{
    if( !node ) return NULL;

    node_t *parent = node->parent;
    while( parent && parent->left == node )
    {
        node   = parent;
        parent = node->parent;
    }

    return node;
}
//------------------------------------------------------------------------------
static
node_t* node_get_grandp(node_t *node)
{
    return ( node && node->parent )?( node->parent->parent ):( NULL );
}
//------------------------------------------------------------------------------
static
node_t* node_get_uncle(node_t *node)
{
    if( !node ) return NULL;

    node_t *parent = node->parent;
    if( !parent ) return NULL;

    node_t *grandp = parent->parent;
    if( !grandp ) return NULL;

    return ( parent == grandp->left )?( grandp->right ):( grandp->left );
}
//------------------------------------------------------------------------------
static
node_t* node_get_brother(node_t *parent, node_t *node)
{
    // The input parent is used for the case if node is NULL.
    if( !parent ) return NULL;
    return ( parent->left == node )?( parent->right ):( parent->left );
}
//------------------------------------------------------------------------------
//---- Map Node - Visit - In Order ---------------------------------------------
//------------------------------------------------------------------------------
static
node_t* tree_get_first_inorder(node_t *root)
{
    return node_get_leftmost_child(root);
}
//------------------------------------------------------------------------------
static
node_t* tree_get_last_inorder(node_t *root)
{
    return node_get_rightmost_child(root);
}
//------------------------------------------------------------------------------
static
node_t* node_get_next_inorder(node_t *node)
{
    if( !node ) return NULL;

    return ( node->right                            )?
           ( node_get_leftmost_child (node->right)  ):
           ( node_get_leftmost_parent(node)->parent );
}
//------------------------------------------------------------------------------
static
node_t* node_get_prev_inorder(node_t *node)
{
    if( !node ) return NULL;

    return ( node->left                              )?
           ( node_get_rightmost_child (node->left)   ):
           ( node_get_rightmost_parent(node)->parent );
}
//------------------------------------------------------------------------------
//---- Map Node - Visit - Post Order -------------------------------------------
//------------------------------------------------------------------------------
static
node_t* tree_get_first_postorder(node_t *root)
{
    node_t *node = root;
    while( node_have_child(node) )
        node = node->left ? node->left : node->right;

    return node;
}
//------------------------------------------------------------------------------
static
node_t* node_get_next_postorder(node_t *node)
{
    if( !node || !node->parent ) return NULL;

    node_t *parent = node->parent;
    return ( parent->right && parent->right != node  )?
           ( tree_get_first_postorder(parent->right) ):
           ( parent                                  );
}
//------------------------------------------------------------------------------
//---- Map Node - Node Link ----------------------------------------------------
//------------------------------------------------------------------------------
static
void node_link_left(node_t *main, node_t *node)
{
    assert( main );
    assert( !main->left );
    if( !node ) return;

    assert( !node->parent );
    main->left   = node;
    node->parent = main;
}
//------------------------------------------------------------------------------
static
void node_link_right(node_t *main, node_t *node)
{
    assert( main );
    assert( !main->right );
    if( !node ) return;

    assert( !node->parent );
    main->right  = node;
    node->parent = main;
}
//------------------------------------------------------------------------------
static
void node_unlink_left(node_t *main)
{
    assert( main );

    node_t *node = main->left;
    if( node )
    {
        main->left   = NULL;
        node->parent = NULL;
    }
}
//------------------------------------------------------------------------------
static
void node_unlink_right(node_t *main)
{
    assert( main );

    node_t *node = main->right;
    if( node )
    {
        main->right  = NULL;
        node->parent = NULL;
    }
}
//------------------------------------------------------------------------------
static
node_t* node_move_parent(node_t *root, node_t *from, node_t *to)
{
    assert( from );
    assert( !to || !to->parent );

    node_t *parent = from->parent;
    if( parent )
    {
        if( from == parent->left )
            parent->left = to;
        else
            parent->right = to;

        from->parent = NULL;
        if( to ) to->parent = parent;
    }
    else
    {
        assert( root == from );
        root = to;
    }

    return root;
}
//------------------------------------------------------------------------------
//---- Map Node - Node Link - Advance ------------------------------------------
//------------------------------------------------------------------------------
static
node_t* tree_replace_node(node_t *root, node_t *node_old, node_t *node_new)
{
    assert( root && node_old && node_new );
    assert( node_is_isolated(node_new) );

    node_t *left = node_old->left;
    node_unlink_left(node_old);
    node_link_left(node_new, left);

    node_t *right = node_old->right;
    node_unlink_right(node_old);
    node_link_right(node_new, right);

    root = node_move_parent(root, node_old, node_new);

    assert( node_is_isolated(node_old) );
    return root;
}
//------------------------------------------------------------------------------
static
node_t* tree_swap_node_pos(node_t *root, node_t *node1, node_t *node2)
{
    assert( root && node1 && node2 );

    node_t temp;
    memset(&temp, 0, sizeof(temp));

    root = tree_replace_node(root, node1, &temp);
    root = tree_replace_node(root, node2, node1);
    root = tree_replace_node(root, &temp, node2);

    return root;
}
//------------------------------------------------------------------------------
static
void tree_swap_node_color(node_t *node1, node_t *node2)
{
    assert( node1 && node2 );

    bool temp = node1->red;
    node1->red = node2->red;
    node2->red = temp;
}
//------------------------------------------------------------------------------
static
node_t* tree_rotate_node_left(node_t *root, node_t *node)
{
    assert( node && node->right );

    node_t *nodel = node;
    node_t *noder = node->right;
    node_t *child = noder->left;

    node_unlink_left (noder);
    node_unlink_right(nodel);
    root = node_move_parent(root, nodel, noder);
    node_link_left (noder, nodel);
    node_link_right(nodel, child);

    return root;
}
//------------------------------------------------------------------------------
static
node_t* tree_rotate_node_right(node_t *root, node_t *node)
{
    assert( node && node->left );

    node_t *nodel = node->left;
    node_t *noder = node;
    node_t *child = nodel->right;

    node_unlink_right(nodel);
    node_unlink_left (noder);
    root = node_move_parent(root, noder, nodel);
    node_link_right(nodel, noder);
    node_link_left (noder, child);

    return root;
}
//------------------------------------------------------------------------------
//---- Map Node - Tree Search --------------------------------------------------
//------------------------------------------------------------------------------
static
node_t* tree_find_closest(node_t *root, const void *tag)
{
    node_t *node = root;
    while( node )
    {
        int cmpres = node->events->tagcmp(tag, node->tag);
        if( cmpres < 0 )
        {
            if( !node->left ) break;
            node = node->left;
        }
        else if( cmpres > 0 )
        {
            if( !node->right ) break;
            node = node->right;
        }
        else
        {
            break;
        }
    }

    return node;
}
//------------------------------------------------------------------------------
static
node_t* tree_find_match(node_t *root, const void *tag)
{
    node_t *node = root;
    while( node )
    {
        int cmpres = node->events->tagcmp(tag, node->tag);
        if( cmpres < 0 )
        {
            if( !node->left ) break;
            node = node->left;
        }
        else if( cmpres > 0 )
        {
            if( !node->right ) break;
            node = node->right;
        }
        else
        {
            return node;
        }
    }

    return NULL;
}
//------------------------------------------------------------------------------
//---- Map Node - Tree Insert Node ---------------------------------------------
//------------------------------------------------------------------------------
static
node_t* tree_insert_adjust(node_t *root, node_t *node)
{
    assert( root && node );

    node_t *parent = node->parent;
    node_t *grandp = node_get_grandp(node);
    node_t *uncle  = node_get_uncle (node);

    if( !parent )
    {
        node->red = false;
    }
    else if( !parent->red )
    {
        // Nothing to do.
    }
    else if( uncle && uncle->red )
    {
        parent->red = false;
        uncle ->red = false;
        grandp->red = true;
        root = tree_insert_adjust(root, grandp);
    }
    else
    {
        if( node == parent->right && parent == grandp->left )
        {
            root = tree_rotate_node_left(root, parent);
            node = node->left;
        }
        else if( node == parent->left && parent == grandp->right )
        {
            root = tree_rotate_node_right(root, parent);
            node = node->right;
        }

        parent = node->parent;
        grandp = node_get_grandp(node);
        uncle  = node_get_uncle (node);

        parent->red = false;
        grandp->red = true;

        if( node == parent->left && parent == grandp->left )
        {
            root = tree_rotate_node_right(root, grandp);
        }
        else if( node == parent->right && parent == grandp->right )
        {
            root = tree_rotate_node_left(root, grandp);
        }
    }

    return root;
}
//------------------------------------------------------------------------------
static
node_t* tree_insert_node(node_t              *root,
                         void                *tag,
                         void                *value,
                         const gmap_events_t *events,
                         bool        /*out*/ *update_existed)
{
    assert( events && update_existed );

    *update_existed = false;

    node_t *node_closest = tree_find_closest(root, tag);
    if( !node_closest )
    {
        root = node_create(tag, value, events);
        root = tree_insert_adjust(root, root);
    }
    else
    {
        int cmpres = events->tagcmp(tag, node_closest->tag);
        if( cmpres < 0 )
        {
            node_t *node = node_create(tag, value, events);
            node_link_left(node_closest, node);
            root = tree_insert_adjust(root, node);
        }
        else if( cmpres > 0 )
        {
            node_t *node = node_create(tag, value, events);
            node_link_right(node_closest, node);
            root = tree_insert_adjust(root, node);
        }
        else
        {
            *update_existed = true;
            node_replace_tag_value(node_closest, tag, value);
        }
    }

    return root;
}
//------------------------------------------------------------------------------
//---- Map Node - Tree Erase Node ----------------------------------------------
//------------------------------------------------------------------------------
static
node_t* node_replace_by_child(node_t* root, node_t* node)
{
    assert( node && !node_have_full_child(node) );

    node_t *child = NULL;
    if( node->left )
    {
        child = node->left;
        node_unlink_left(node);
    }
    else if( node->right )
    {
        child = node->right;
        node_unlink_right(node);
    }

    root = node_move_parent(root, node, child);

    return root;
}
//------------------------------------------------------------------------------
static
node_t* tree_erase_adjust(node_t *root, node_t *parent, node_t *node)
{
    if( !parent ) return root;

    node_t *brother  = node_get_brother(parent, node);
    node_t *broleft  = brother ? brother->left  : NULL;
    node_t *broright = brother ? brother->right : NULL;

    if( node_is_red(brother) )
    {
        parent ->red = true;
        brother->red = false;
        if( node == parent->left )
            root = tree_rotate_node_left(root, parent);
        else
            root = tree_rotate_node_right(root, parent);
    }

    brother  = node_get_brother(parent, node);
    broleft  = brother ? brother->left  : NULL;
    broright = brother ? brother->right : NULL;
    assert( brother );

    if( node_is_black(parent  ) &&
        node_is_black(brother ) &&
        node_is_black(broleft ) &&
        node_is_black(broright) )
    {
        brother->red = true;
        root = tree_erase_adjust(root, parent->parent, parent);
    }
    else if( node_is_red  (parent  ) &&
             node_is_black(brother ) &&
             node_is_black(broleft ) &&
             node_is_black(broright) )
    {
        brother->red = true;
        parent ->red = false;;
    }
    else
    {
        if( node == parent->left    &&
            node_is_black(brother ) &&
            node_is_red  (broleft ) &&
            node_is_black(broright) )
        {
            brother->red = true;
            broleft->red = false;
            root = tree_rotate_node_right(root, brother);
        }
        else if( node == parent->right   &&
                 node_is_black(brother ) &&
                 node_is_black(broleft ) &&
                 node_is_red  (broright) )
        {
            brother ->red = true;
            broright->red = false;
            root = tree_rotate_node_left(root, brother);
        }

        brother  = node_get_brother(parent, node);
        broleft  = brother ? brother->left  : NULL;
        broright = brother ? brother->right : NULL;
        assert( brother );

        brother->red = parent->red;
        parent ->red = false;

        if( node == parent->left )
        {
            assert( broright );
            broright->red = false;
            root = tree_rotate_node_left(root, parent);
        }
        else
        {
            assert( broleft );
            broleft->red = false;
            root = tree_rotate_node_right(root, parent);
        }
    }

    return root;
}
//------------------------------------------------------------------------------
static
node_t* tree_erase_node(node_t *root, node_t *node)
{
    if( !node ) return root;

    // Exchange node with the nearest single (or none) child node
    if( node_have_full_child(node) )
    {
        node_t *nearest = node_get_rightmost_child(node->left);
        root = tree_swap_node_pos(root, node, nearest);
        tree_swap_node_color(node, nearest);
    }

    // Extract the node and adjust the rest
    node_t *child  = node->left ? node->left : node->right;
    node_t *parent = node->parent;
    root = node_replace_by_child(root, node);

    if( node_is_red(node) )
    {
        // Nothing to do.
    }
    else if( node_is_red(child) )
    {
        child->red = false;
    }
    else
    {
        root = tree_erase_adjust(root, parent, child);
    }

    // Release the node
    node_release(node);

    return root;
}
//------------------------------------------------------------------------------
//---- Map Node - Tree Release All ---------------------------------------------
//------------------------------------------------------------------------------
static
void tree_release_all_nodes(node_t *root)
{
    node_t *node = tree_get_first_postorder(root);
    while( node )
    {
        node_t *nodrm = node;
        node = node_get_next_postorder(node);

        node_release(nodrm);
    }
}
//------------------------------------------------------------------------------
//---- Iterator ----------------------------------------------------------------
//------------------------------------------------------------------------------
static
void gmap_iter_init(gmap_iter_t *iter, const gmap_t *map, node_t *node)
{
    assert( iter );

    iter->container = map;
    iter->node      = node;
}
//------------------------------------------------------------------------------
bool gmap_iter_is_available(const gmap_iter_t *iter)
{
    /**
     * @memberof gmap_iter_t
     * @brief Check if the iterator is available.
     *
     * @param iter Object instance.
     * @return TRUE if it is available; and FALSE if not.
     */
    assert( iter );
    return iter->node;
}
//------------------------------------------------------------------------------
bool gmap_iter_move_prev(gmap_iter_t *iter)
{
    /**
     * @memberof gmap_iter_t
     * @brief Move iterator to the previous position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = node_get_prev_inorder(iter->node) );
}
//------------------------------------------------------------------------------
bool gmap_iter_move_next(gmap_iter_t *iter)
{
    /**
     * @memberof gmap_iter_t
     * @brief Move iterator to the next position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = node_get_next_inorder(iter->node) );
}
//------------------------------------------------------------------------------
const void* gmap_iter_get_tag(const gmap_iter_t *iter)
{
    /**
     * @memberof gmap_iter_t
     * @brief Get the tag located by this iterator.
     *
     * @param iter Object instance.
     * @return The tag located by this iterator,
     *         or NULL if there have no item.
     */
    assert( iter );
    return iter->node ? iter->node->tag : NULL;
}
//------------------------------------------------------------------------------
void* gmap_iter_get_value(gmap_iter_t *iter)
{
    /**
     * @memberof gmap_iter_t
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
bool gmap_iter_set_value(gmap_iter_t *iter, void *value)
{
    /**
     * @memberof gmap_iter_t
     * @brief Set the item located by this iterator.
     *
     * @param iter  Object instance.
     * @param value An item to replace the old one.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );

    node_t *node = iter->node;
    if( !node ) return false;

    node->events->itemfree(node->value);
    node->value = value;

    return true;
}
//------------------------------------------------------------------------------
//---- Constant Iterator -------------------------------------------------------
//------------------------------------------------------------------------------
static
void gmap_citer_init(gmap_citer_t *iter, const gmap_t *map, const node_t *node)
{
    assert( iter );

    iter->container = map;
    iter->node      = node;
}
//------------------------------------------------------------------------------
bool gmap_citer_is_available(const gmap_citer_t *iter)
{
    /**
     * @memberof gmap_citer_t
     * @brief Check if the iterator is available.
     *
     * @param iter Object instance.
     * @return TRUE if it is available; and FALSE if not.
     */
    assert( iter );
    return iter->node;
}
//------------------------------------------------------------------------------
bool gmap_citer_move_prev(gmap_citer_t *iter)
{
    /**
     * @memberof gmap_citer_t
     * @brief Move iterator to the previous position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = node_get_prev_inorder((node_t*)iter->node) );
}
//------------------------------------------------------------------------------
bool gmap_citer_move_next(gmap_citer_t *iter)
{
    /**
     * @memberof gmap_citer_t
     * @brief Move iterator to the next position.
     *
     * @param iter Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( iter );
    return ( iter->node = node_get_next_inorder((node_t*)iter->node) );
}
//------------------------------------------------------------------------------
const void* gmap_citer_get_tag(const gmap_citer_t *iter)
{
    /**
     * @memberof gmap_citer_t
     * @brief Get the tag located by this iterator.
     *
     * @param iter Object instance.
     * @return The tag located by this iterator,
     *         or NULL if there have no tag.
     */
    assert( iter );
    return iter->node ? iter->node->tag : NULL;
}
//------------------------------------------------------------------------------
const void* gmap_citer_get_value(const gmap_citer_t *iter)
{
    /**
     * @memberof gmap_citer_t
     * @brief Get the value located by this iterator.
     *
     * @param iter Object instance.
     * @return The value located by this iterator,
     *         or NULL if there have no value.
     */
    assert( iter );
    return iter->node ? iter->node->value : NULL;
}
//------------------------------------------------------------------------------
//---- Map Class ---------------------------------------------------------------
//------------------------------------------------------------------------------
static
int gmap_tagcmp_default(const void *tag1, const void *tag2)
{
    uintptr_t value1 = (uintptr_t)tag1;
    uintptr_t value2 = (uintptr_t)tag2;
    if     ( value1 < value2 ) return -1;
    else if( value1 > value2 ) return 1;
    else                       return 0;
}
//------------------------------------------------------------------------------
static
void gmap_tagfree_default(void *tag)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
static
void gmap_itemfree_default(void *item)
{
    // Nothing to do.
}
//------------------------------------------------------------------------------
void gmap_init(gmap_t *map, gmap_tagcmp_t   tagcmp,
                            gmap_tagfree_t  tagfree,
                            gmap_itemfree_t itemfree)
{
    /**
     * @memberof gmap_t
     * @brief Constructor.
     *
     * @param map      Object instance.
     * @param tagcmp   The function to compare two tags.
     *                 The tags will be compared as they are integral type if this parameter is NULL.
     * @param tagfree  The function used to release a tag.
     *                 This parameter can be NULL if not needed.
     * @param itemfree The function used to release an item (value).
     *                 This parameter can be NULL if not needed.
     */
    assert( map );

    tagcmp   = tagcmp   ? tagcmp   : gmap_tagcmp_default;
    tagfree  = tagfree  ? tagfree  : gmap_tagfree_default;
    itemfree = itemfree ? itemfree : gmap_itemfree_default;

    map->root   = NULL;
    map->count  = 0;
    map->events = gmap_events_create(tagcmp, tagfree, itemfree);
}
//------------------------------------------------------------------------------
void gmap_init_movefrom(gmap_t *map, gmap_t *src)
{
    /**
     * @memberof gmap_t
     * @brief Construct, and move data from another container.
     *
     * @param map Object instance.
     * @param src Another container object to move data from.
     */
    assert( map && src );

    gmap_init(map, NULL, NULL, NULL);
    gmap_movefrom(map, src);
}
//------------------------------------------------------------------------------
void gmap_deinit(gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Destructor.
     *
     * @param map Object instance.
     */
    assert( map );

    gmap_clear(map);
    gmap_events_release(map->events);
}
//------------------------------------------------------------------------------
gmap_iter_t gmap_get_first(gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Get the first value.
     *
     * @param map Object instance.
     * @return An iterator of the first value,
     *         and that may be an invalid iterator if there have no any value.
     */
    assert( map );

    gmap_iter_t iter;
    gmap_iter_init(&iter, map, tree_get_first_inorder(map->root));

    return iter;
}
//------------------------------------------------------------------------------
gmap_iter_t gmap_get_last(gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Get the last value.
     *
     * @param map Object instance.
     * @return An iterator of the last value,
     *         and that may be an invalid iterator if there have no any value.
     */
    assert( map );

    gmap_iter_t iter;
    gmap_iter_init(&iter, map, tree_get_last_inorder(map->root));

    return iter;
}
//------------------------------------------------------------------------------
gmap_citer_t gmap_get_cfirst(const gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Get the first value.
     *
     * @param map Object instance.
     * @return A constant iterator of the first value,
     *         and that may be an invalid iterator if there have no any value.
     */
    assert( map );

    gmap_citer_t iter;
    gmap_citer_init(&iter, map, tree_get_first_inorder(map->root));

    return iter;
}
//------------------------------------------------------------------------------
gmap_citer_t gmap_get_clast(const gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Get the last value.
     *
     * @param map Object instance.
     * @return A constant iterator of the last value,
     *         and that may be an invalid iterator if there have no any value.
     */
    assert( map );

    gmap_citer_t iter;
    gmap_citer_init(&iter, map, tree_get_last_inorder(map->root));

    return iter;
}
//------------------------------------------------------------------------------
gmap_iter_t gmap_find(gmap_t *map, const void *tag)
{
    /**
     * @memberof gmap_t
     * @brief Find a value by tag.
     *
     * @param map Object instance.
     * @param tag The specific tag to find value.
     * @return An iterator of the value which corresponding to the tag,
     *         and that may be an invalid iterator if not found.
     */
    assert( map );

    gmap_iter_t pos;
    gmap_iter_init(&pos, map, tree_find_match(map->root, tag));

    return pos;
}
//------------------------------------------------------------------------------
gmap_citer_t gmap_cfind(const gmap_t *map, const void *tag)
{
    /**
     * @memberof gmap_t
     * @brief Find a value by tag.
     *
     * @param map Object instance.
     * @param tag The specific tag to find value.
     * @return A constant iterator of the value which corresponding to the tag,
     *         and that may be an invalid iterator if not found.
     */
    assert( map );

    gmap_citer_t pos;
    gmap_citer_init(&pos, map, tree_find_match(map->root, tag));

    return pos;
}
//------------------------------------------------------------------------------
void* gmap_find_item(gmap_t *map, const void *tag)
{
    /**
     * @memberof gmap_t
     * @brief Find value by tag.
     *
     * @param map Object instance.
     * @param tag The specific tag to find value.
     * @return The item value of that tag; or NULL if not found.
     */
    assert( map );

    gmap_iter_t pos = gmap_find(map, tag);
    return gmap_iter_get_value(&pos);
}
//------------------------------------------------------------------------------
const void* gmap_find_citem(const gmap_t *map, const void *tag)
{
    /**
     * @memberof gmap_t
     * @brief Find value by tag.
     *
     * @param map Object instance.
     * @param tag The specific tag to find value.
     * @return The item value of that tag; or NULL if not found.
     */
    assert( map );

    gmap_citer_t pos = gmap_cfind(map, tag);
    return gmap_citer_get_value(&pos);
}
//------------------------------------------------------------------------------
void gmap_insert(gmap_t *map, void *tag, void *value)
{
    /**
     * @memberof gmap_t
     * @brief Insert a value.
     *
     * @param map   Object instance.
     * @param tag   Tag of the value.
     * @param value The value to be added to the container.
     */
    assert( map );

    bool update_existed;
    map->root = tree_insert_node(map->root, tag, value, map->events, &update_existed);
    if( !update_existed ) ++map->count;
}
//------------------------------------------------------------------------------
void gmap_erase(gmap_t *map, gmap_iter_t *pos)
{
    /**
     * @memberof gmap_t
     * @brief Erase a value by a specific position.
     *
     * @param map Object instance.
     * @param pos Position of the value to be erased.
     */
    assert( map && pos );
    assert( pos->container == map );

    node_t *node = pos->node;
    if( !node ) return;

    assert( map->count );
    map->root = tree_erase_node(map->root, node);
    --map->count;
}
//------------------------------------------------------------------------------
void gmap_erase_bytag(gmap_t *map, const void *tag)
{
    /**
     * @memberof gmap_t
     * @brief Erase a value by a specific tag.
     *
     * @param map Object instance.
     * @param tag Tag of the value to be erased.
     */
    assert( map );

    gmap_iter_t pos = gmap_find(map, tag);
    gmap_erase(map, &pos);
}
//------------------------------------------------------------------------------
void gmap_clear(gmap_t *map)
{
    /**
     * @memberof gmap_t
     * @brief Erase all values.
     *
     * @param map Object instance.
     */
    assert( map );

    tree_release_all_nodes(map->root);
    map->root  = NULL;
    map->count = 0;
}
//------------------------------------------------------------------------------
void gmap_movefrom(gmap_t *map, gmap_t *src)
{
    /**
     * @memberof gmap_t
     * @brief Move and import data from another container.
     *
     * @param map Object instance.
     * @param src The data source.
     *
     * @remarks All old data stored in this container will be erased.
     */
    assert( map && src );

    gmap_clear(map);

    map->root  = src->root;
    map->count = src->count;

    src->root  = NULL;
    src->count = 0;

    *map->events = *src->events;
}
//------------------------------------------------------------------------------

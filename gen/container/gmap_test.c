#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#include "map.h"

//------------------------------------------------------------------------------
//---- Test Object -------------------------------------------------------------
//------------------------------------------------------------------------------
int testobj_refcnt = 0;
//------------------------------------------------------------------------------
typedef struct testobj_t
{
    int value;
} testobj_t;
//------------------------------------------------------------------------------
testobj_t* testobj_create(int value)
{
    ++testobj_refcnt;

    testobj_t *obj = malloc(sizeof(testobj_t));
    assert( obj );

    obj->value = value;

    return obj;
}
//------------------------------------------------------------------------------
void testobj_release(testobj_t *obj)
{
    assert( obj );

    --testobj_refcnt;
    free(obj);
}
//------------------------------------------------------------------------------
//---- Tree Check and Test -----------------------------------------------------
//------------------------------------------------------------------------------
void print_rbtree_errmsg(const char *funcname, const void *tag, const char *msg)
{
    printf("RB TEST FAILED in %s : tag %d, %s\n", funcname, (int)(intptr_t)tag, msg);
}
#define PRINT_RBTREE_ERRMSG(node,msg) print_rbtree_errmsg(__func__, node->tag, msg)
//------------------------------------------------------------------------------
bool rbtree_propchk_link(gmap_node_t *node)
{
    if( !node ) return true;

    if( node->left && node->left->parent != node )
    {
        PRINT_RBTREE_ERRMSG(node, "Node link abnormal!");
        return false;
    }
    if( node->right && node->right->parent != node )
    {
        PRINT_RBTREE_ERRMSG(node, "Node link abnormal!");
        return false;
    }

    return rbtree_propchk_link(node->left ) &&
           rbtree_propchk_link(node->right);
}
//------------------------------------------------------------------------------
bool rbtree_propchk_tagvalue(gmap_node_t *node)
{
    if( !node ) return true;

    if( node->left && node->tag <= node->left->tag )
    {
        PRINT_RBTREE_ERRMSG(node, "Tags out of sequence!");
        return false;
    }
    if( node->right && node->tag >= node->right->tag )
    {
        PRINT_RBTREE_ERRMSG(node, "Tags out of sequence!");
        return false;
    }

    return rbtree_propchk_tagvalue(node->left ) &&
           rbtree_propchk_tagvalue(node->right);
}
//------------------------------------------------------------------------------
bool rbtree_propchk_root(gmap_node_t *root)
{
    if( root && root->red )
    {
        PRINT_RBTREE_ERRMSG(root, "Root is not black!");
        return false;
    }
    else
    {
        return true;
    }
}
//------------------------------------------------------------------------------
bool rbtree_propchk_reds(gmap_node_t *node)
{
    if( !node ) return true;

    if( node->red )
    {
        if( node->left && node->left->red )
        {
            PRINT_RBTREE_ERRMSG(node, "Red node have red child!");
            return false;
        }
        if( node->right && node->right->red )
        {
            PRINT_RBTREE_ERRMSG(node, "Red node have red child!");
            return false;
        }
    }

    return rbtree_propchk_reds(node->left ) &&
           rbtree_propchk_reds(node->right);
}
//------------------------------------------------------------------------------
size_t rbtree_propchk_blackscnt(gmap_node_t *node, bool *res)
{
    *res = true;

    if( !node ) return 1;

    bool chkl, chkr;
    size_t cntl = rbtree_propchk_blackscnt(node->left , &chkl);
    size_t cntr = rbtree_propchk_blackscnt(node->right, &chkr);
    if( cntl != cntr )
    {
        PRINT_RBTREE_ERRMSG(node, "Black counts not match in path!");
        *res = false;
    }
    else
    {
        *res = chkl && chkr;
    }

    return cntl + ( node->red ? 0 : 1 );
}
//------------------------------------------------------------------------------
size_t rbtree_count_nodes(gmap_node_t *node)
{
    if( !node ) return 0;
    return rbtree_count_nodes(node->left) + rbtree_count_nodes(node->right) + 1;
}
//------------------------------------------------------------------------------
bool rbtree_propchk(gmap_node_t *root)
{
    bool chk1 = rbtree_propchk_link     (root) &&
                rbtree_propchk_tagvalue (root) &&
                rbtree_propchk_root     (root) &&
                rbtree_propchk_reds     (root);
    bool chk2;
    rbtree_propchk_blackscnt(root, &chk2);

    return chk1 && chk2;
}
//------------------------------------------------------------------------------
bool rbtree_countchk(gmap_node_t *root, size_t count_target)
{
    bool res = rbtree_count_nodes(root) == count_target;
    if( !res ) printf("TEST FAILED in %s : %s\n", __func__, "Container elements count not match!");

    return res;
}
//------------------------------------------------------------------------------
bool gmap_totalchk(gmap_t *map)
{
    return rbtree_propchk (map->root) &&
           rbtree_countchk(map->root, map->count);
}
//------------------------------------------------------------------------------
//---- Main Test Process -------------------------------------------------------
//------------------------------------------------------------------------------
bool map_insert_and_erase_test(const int *tags_insert, const int *tags_erase, unsigned tags_count)
{
    assert( tags_insert && tags_erase );

    gmap_t map;
    gmap_init(&map, NULL, NULL, (gmap_itemfree_t)testobj_release);

    assert( gmap_get_count(&map) == 0 );
    assert( testobj_refcnt == 0 );

    unsigned i;

    if( !gmap_totalchk(&map) ) return false;
    for(i=0; i<tags_count; ++i)
    {
        gmap_insert(&map, (void*)(intptr_t)tags_insert[i], testobj_create(tags_insert[i]));
        if( !gmap_totalchk(&map) ) return false;
    }

    if( !gmap_totalchk(&map) ) return false;
    for(i=0; i<tags_count; ++i)
    {
        gmap_erase_bytag(&map, (void*)(intptr_t)tags_erase[i]);
        if( !gmap_totalchk(&map) ) return false;
    }

    gmap_deinit(&map);
    assert( testobj_refcnt == 0 );

    return true;
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Insert and erase test

    {
        int tags_insert[] = {31,32,21,12,25,16,9,13,11,27,26,7,5,6,28,18,17,22,19,29,30,2,23,15,20,3,1,10,14,8,24,4};
        int tags_erase [] = {27,32,15,19,26,1,9,22,3,21,29,28,13,23,7,11,17,31,14,5,12,4,6,20,30,10,16,18,8,25,24,2};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    {
        int tags_insert[] = {27,32,25,17,21,7,10,12,22,14,16,3,1,15,29,5,2,6,4,9,24,26,8,30,31,18,11,19,20,28,13,23};
        int tags_erase [] = {31,10,6,19,27,1,5,15,2,28,3,20,30,24,18,16,25,17,26,29,32,7,11,4,23,9,8,14,22,13,21,12};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    {
        int tags_insert[] = {2,21,1,28,24,22,13,5,10,30,29,8,4,11,9,19,3,14,6,7,15,25,23,26,17,16,32,18,31,27,20,12};
        int tags_erase [] = {26,17,2,14,28,18,3,12,30,21,13,22,32,4,5,1,24,19,25,16,23,9,10,11,29,15,27,7,20,31,8,6};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    {
        int tags_insert[] = {7,17,29,2,22,30,24,19,4,26,31,27,16,32,21,11,3,12,9,10,23,5,20,25,1,28,8,18,14,13,15,6};
        int tags_erase [] = {20,1,29,21,31,32,24,27,22,3,4,9,25,6,13,30,17,5,28,8,15,18,2,11,16,10,12,19,26,23,7,14};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    {
        int tags_insert[] = {15,1,10,20,2,21,3,13,9,30,26,17,5,7,29,27,24,4,31,11,8,16,28,19,32,14,12,18,23,22,6,25};
        int tags_erase [] = {26,14,15,12,29,13,1,20,19,22,9,6,24,7,5,2,4,10,16,21,23,28,30,17,27,18,31,8,25,3,32,11};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    {
        int tags_insert[] = {15,26,6,23,27,18,32,31,11,3,8,24,17,13,16,25,4,19,28,1,10,9,5,30,14,29,2,22,12,20,7,21};
        int tags_erase [] = {31,13,29,24,22,20,4,25,11,16,5,9,19,8,18,23,7,12,32,26,27,14,6,3,17,30,2,10,15,21,28,1};
        assert( sizeof(tags_insert) == sizeof(tags_erase) );
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);

        assert( map_insert_and_erase_test(tags_insert, tags_erase, tags_count) );
    }

    // Duplicated insert and erase test

    {
        gmap_t map;
        gmap_init(&map, NULL, NULL, (gmap_itemfree_t)testobj_release);

        int tags[] = {15,   6,   27,   32,   11,  8,   17,   16,   4,   28,  10,  5,   14,   2,   12,   7,
                      15,26,6,23,27,18,32,31,11,3,8,24,17,13,16,25,4,19,28,1,10,9,5,30,14,29,2,22,12,20,7,21};
        int tags_count = sizeof(tags)/sizeof(tags[0]);
        int real_count = 32;

        // Insert
        int i;
        for(i=0; i<tags_count; ++i)
        {
            gmap_insert(&map, (void*)(intptr_t)tags[i], testobj_create(tags[i]));
            assert( gmap_totalchk(&map) );
        }
        assert( gmap_get_count(&map) == real_count );

        // Erase
        for(i=0; i<tags_count; ++i)
        {
            gmap_erase_bytag(&map, (void*)(intptr_t)tags[i]);
            assert( gmap_totalchk(&map) );
        }
        assert( gmap_get_count(&map) == 0 );

        gmap_deinit(&map);
        assert( testobj_refcnt == 0 );
    }

    // Clear all elements test

    {
        gmap_t map;
        gmap_init(&map, NULL, NULL, (gmap_itemfree_t)testobj_release);

        int tags_insert[] = {31,32,21,12,25,16,9,13,11,27,26,7,5,6,28,18,17,22,19,29,30,2,23,15,20,3,1,10,14,8,24,4};
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);
        int i;
        for(i=0; i<tags_count; ++i)
            gmap_insert(&map, (void*)(intptr_t)tags_insert[i], testobj_create(tags_insert[i]+1000));

        gmap_clear(&map);
        assert( testobj_refcnt == 0 );

        gmap_deinit(&map);
        assert( testobj_refcnt == 0 );
    }

    // Iterator read test

    {
        gmap_t map;
        gmap_init(&map, NULL, NULL, (gmap_itemfree_t)testobj_release);

        // Build data
        int tags_insert[] = {31,32,21,12,25,16,9,13,11,27,26,7,5,6,28,18,17,22,19,29,30,2,23,15,20,3,1,10,14,8,24,4};
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);
        int i;
        for(i=0; i<tags_count; ++i)
            gmap_insert(&map, (void*)(intptr_t)tags_insert[i], testobj_create(tags_insert[i]+1000));

        // Get and check each elements
        gmap_citer_t iter = gmap_get_cfirst(&map);
        for(i=0; i<tags_count; ++i, gmap_citer_move_next(&iter))
        {
            int tag   = i   + 1;
            int value = tag + 1000;

            assert( gmap_citer_is_available(&iter) );

            assert( gmap_citer_get_tag(&iter) == (void*)(intptr_t)tag );

            const testobj_t *item = gmap_citer_get_value(&iter);
            assert( item->value == value );
        }

        gmap_deinit(&map);
        assert( testobj_refcnt == 0 );
    }

    // Iterator modify test

    {
        gmap_t map;
        gmap_init(&map, NULL, NULL, (gmap_itemfree_t)testobj_release);

        // Build data
        int tags_insert[] = {31,32,21,12,25,16,9,13,11,27,26,7,5,6,28,18,17,22,19,29,30,2,23,15,20,3,1,10,14,8,24,4};
        int tags_count = sizeof(tags_insert)/sizeof(tags_insert[0]);
        int i;
        for(i=0; i<tags_count; ++i)
            gmap_insert(&map, (void*)(intptr_t)tags_insert[i], testobj_create(tags_insert[i]+1000));

        // Find element
        int tag = 12;
        gmap_iter_t iter = gmap_find(&map, (void*)(intptr_t)tag);
        assert( gmap_iter_is_available(&iter) );
        assert( gmap_iter_get_tag(&iter) == (void*)(intptr_t)tag );

        // Modify element value
        int newvalue = 1024;
        assert( gmap_iter_set_value(&iter, testobj_create(newvalue)) );

        // Check modify
        iter = gmap_find(&map, (void*)(intptr_t)tag);
        assert( gmap_iter_is_available(&iter) );
        assert( gmap_iter_get_tag(&iter) == (void*)(intptr_t)tag );
        testobj_t *item = gmap_iter_get_value(&iter);
        assert( item->value == newvalue );

        gmap_deinit(&map);
        assert( testobj_refcnt == 0 );
    }

    return 0;
}
//------------------------------------------------------------------------------

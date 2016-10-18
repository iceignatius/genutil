#include <assert.h>
#include <stdlib.h>

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#include "list.h"

int testobj_refcnt = 0;

typedef struct testobj_t
{
    int value;
} testobj_t;

testobj_t* testobj_create(int value)
{
    ++testobj_refcnt;

    testobj_t *obj = malloc(sizeof(testobj_t));
    assert( obj );

    obj->value = value;

    return obj;
}

void testobj_release(testobj_t *obj)
{
    assert( obj );

    --testobj_refcnt;
    free(obj);
}

bool verify_list_values(const glist_t *list, const int *target_arr, size_t count)
{
    assert( list );

    if( glist_get_count(list) != count ) return false;

    glist_citer_t iter;
    int i;
    for(i=0, iter=glist_get_cfirst(list);
        i<count;
        ++i, glist_citer_move_next(&iter))
    {
        if( !glist_citer_is_available(&iter) ) return false;

        const testobj_t *item = glist_citer_get_value(&iter);
        if( !item || item->value != target_arr[i] ) return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    glist_t list;
    glist_init(&list, (glist_itemfree_t)testobj_release);

    assert( glist_get_count(&list) == 0 );
    assert( testobj_refcnt == 0 );

    // Front and back push test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_push_back (&list, testobj_create(2));
        glist_push_back (&list, testobj_create(4));
        glist_push_front(&list, testobj_create(7));
        glist_push_front(&list, testobj_create(5));
        glist_push_back (&list, testobj_create(6));
        glist_push_back (&list, testobj_create(8));
        glist_push_front(&list, testobj_create(3));
        glist_push_front(&list, testobj_create(1));

        assert( glist_get_count(&list) == 8 );
        assert( verify_list_values(&list, target, 8) );
    }

    // Front and back pop test
    {
        static const int target[] = { /*1, 3,*/ 5, 7, 2, /*4, 6, 8*/ };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_push_back(&list, testobj_create(1));
        glist_push_back(&list, testobj_create(3));
        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(7));
        glist_push_back(&list, testobj_create(2));
        glist_push_back(&list, testobj_create(4));
        glist_push_back(&list, testobj_create(6));
        glist_push_back(&list, testobj_create(8));

        glist_pop_front(&list);
        glist_pop_front(&list);
        glist_pop_back (&list);
        glist_pop_back (&list);
        glist_pop_back (&list);

        assert( glist_get_count(&list) == 3 );
        assert( verify_list_values(&list, target, 3) );

        glist_pop_front(&list);
        glist_pop_front(&list);
        glist_pop_front(&list);
        glist_pop_front(&list);
        glist_pop_back (&list);
        glist_pop_back (&list);
        glist_pop_back (&list);
        glist_pop_back (&list);

        assert( glist_get_count(&list) == 0 );
        assert( verify_list_values(&list, NULL, 0) );
    }

    // Clear test
    {
        glist_push_back(&list, testobj_create(1));
        glist_push_back(&list, testobj_create(3));
        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(7));
        glist_push_back(&list, testobj_create(2));
        glist_push_back(&list, testobj_create(4));
        glist_push_back(&list, testobj_create(6));
        glist_push_back(&list, testobj_create(8));
        assert( !glist_is_empty(&list) );

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );
    }

    // Insert test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_iter_t iter;

        iter = glist_get_first(&list);
        glist_insert(&list, &iter, testobj_create(8));
        /* 8 */

        iter = glist_get_first(&list);
        glist_insert(&list, &iter, testobj_create(1));
        glist_insert(&list, &iter, testobj_create(3));
        /* 1, 3, 8 */

        iter = glist_get_last(&list);
        glist_insert(&list, &iter, testobj_create(6));
        /* 1, 3, 6, 8 */

        iter = glist_get_first(&list);
        assert( glist_iter_move_next(&iter) );
        assert( glist_iter_move_next(&iter) );
        glist_insert(&list, &iter, testobj_create(5));
        glist_insert(&list, &iter, testobj_create(7));
        /* 1, 3, 5, 7, 6, 8 */

        iter = glist_get_last(&list);
        glist_iter_move_prev(&iter);
        glist_insert(&list, &iter, testobj_create(2));
        glist_insert(&list, &iter, testobj_create(4));
        /* 1, 3, 5, 7, 2, 4, 6, 8 */

        assert( glist_get_count(&list) == 8 );
        assert( verify_list_values(&list, target, 8) );
    }

    // Erase test
    {
        static const int target[] = { /*1,*/ 3, 5, /*7, 2,*/ 4, 6, /*8*/ };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_push_back(&list, testobj_create(1));
        glist_push_back(&list, testobj_create(3));
        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(7));
        glist_push_back(&list, testobj_create(2));
        glist_push_back(&list, testobj_create(4));
        glist_push_back(&list, testobj_create(6));
        glist_push_back(&list, testobj_create(8));

        glist_iter_t iter;

        iter = glist_get_first(&list);
        glist_erase(&list, &iter);

        iter = glist_get_last(&list);
        glist_erase(&list, &iter);

        iter = glist_get_first(&list);
        assert( glist_iter_move_next(&iter) );
        assert( glist_iter_move_next(&iter) );
        glist_iter_t iter2 = iter;
        assert( glist_iter_move_next(&iter2) );
        glist_erase(&list, &iter);
        glist_erase(&list, &iter2);

        assert( glist_get_count(&list) == 4 );
        assert( verify_list_values(&list, target, 4) );

        int i;
        for(i=0; i<4; ++i)
        {
            iter = glist_get_first(&list);
            glist_erase(&list, &iter);
            iter = glist_get_last(&list);
            glist_erase(&list, &iter);
        }
        assert( glist_get_count(&list) == 0 );
        assert( verify_list_values(&list, NULL, 0) );
    }

    // List move test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(5));

        {
            glist_t list2;
            glist_init(&list2, (glist_itemfree_t)testobj_release);

            glist_push_back(&list2, testobj_create(1));
            glist_push_back(&list2, testobj_create(3));
            glist_push_back(&list2, testobj_create(5));
            glist_push_back(&list2, testobj_create(7));
            glist_push_back(&list2, testobj_create(2));
            glist_push_back(&list2, testobj_create(4));
            glist_push_back(&list2, testobj_create(6));
            glist_push_back(&list2, testobj_create(8));

            glist_movefrom(&list, &list2);

            assert( glist_get_count(&list2) == 0 );
            assert( verify_list_values(&list2, NULL, 0) );

            glist_deinit(&list2);
        }

        assert( glist_get_count(&list) == 8 );
        assert( verify_list_values(&list, target, 8) );

        glist_clear(&list);
        assert( testobj_refcnt == 0 );
    }

    // Iterator modify item test
    {
        static const int target[] = { 1, 3, 5, 60, 2, 4, 6, 8 };

        glist_clear(&list);
        assert( glist_is_empty(&list) );
        assert( testobj_refcnt == 0 );

        glist_push_back(&list, testobj_create(1));
        glist_push_back(&list, testobj_create(3));
        glist_push_back(&list, testobj_create(5));
        glist_push_back(&list, testobj_create(7));
        glist_push_back(&list, testobj_create(2));
        glist_push_back(&list, testobj_create(4));
        glist_push_back(&list, testobj_create(6));
        glist_push_back(&list, testobj_create(8));

        glist_iter_t iter = glist_get_first(&list);
        assert( glist_iter_move_next(&iter) );
        assert( glist_iter_move_next(&iter) );
        assert( glist_iter_move_next(&iter) );
        assert( glist_iter_set_value(&iter, testobj_create(60)) );

        assert( glist_get_count(&list) == 8 );
        assert( verify_list_values(&list, target, 8) );

        glist_clear(&list);
        assert( testobj_refcnt == 0 );
    }

    glist_deinit(&list);
    assert( testobj_refcnt == 0 );

    return 0;
}

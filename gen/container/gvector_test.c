#include <assert.h>
#include <stdlib.h>

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#include "vector.h"

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

bool verify_vector_values(const gvector_t *vector, const int *target_arr, size_t count)
{
    assert( vector );

    if( gvector_get_count(vector) != count ) return false;

    int i;
    for(i=0; i<count; ++i)
    {
        const testobj_t *item = gvector_get_citem(vector, i);
        if( !item || item->value != target_arr[i] ) return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    gvector_t vector;
    gvector_init(&vector, (gvector_itemfree_t)testobj_release);

    assert( gvector_get_count(&vector) == 0 );
    assert( testobj_refcnt == 0 );

    // Front and back push test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        gvector_push_back (&vector, testobj_create(2));
        gvector_push_back (&vector, testobj_create(4));
        gvector_push_front(&vector, testobj_create(7));
        gvector_push_front(&vector, testobj_create(5));
        gvector_push_back (&vector, testobj_create(6));
        gvector_push_back (&vector, testobj_create(8));
        gvector_push_front(&vector, testobj_create(3));
        gvector_push_front(&vector, testobj_create(1));

        assert( gvector_get_count(&vector) == 8 );
        assert( verify_vector_values(&vector, target, 8) );
    }

    // Front and back pop test
    {
        static const int target[] = { /*1, 3,*/ 5, 7, 2, /*4, 6, 8*/ };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        gvector_push_back(&vector, testobj_create(1));
        gvector_push_back(&vector, testobj_create(3));
        gvector_push_back(&vector, testobj_create(5));
        gvector_push_back(&vector, testobj_create(7));
        gvector_push_back(&vector, testobj_create(2));
        gvector_push_back(&vector, testobj_create(4));
        gvector_push_back(&vector, testobj_create(6));
        gvector_push_back(&vector, testobj_create(8));

        gvector_pop_front(&vector);
        gvector_pop_front(&vector);
        gvector_pop_back (&vector);
        gvector_pop_back (&vector);
        gvector_pop_back (&vector);

        assert( gvector_get_count(&vector) == 3 );
        assert( verify_vector_values(&vector, target, 3) );

        gvector_pop_front(&vector);
        gvector_pop_front(&vector);
        gvector_pop_front(&vector);
        gvector_pop_front(&vector);
        gvector_pop_back (&vector);
        gvector_pop_back (&vector);
        gvector_pop_back (&vector);
        gvector_pop_back (&vector);

        assert( gvector_get_count(&vector) == 0 );
        assert( verify_vector_values(&vector, NULL, 0) );
    }

    // Clear test
    {
        gvector_push_back(&vector, testobj_create(1));
        gvector_push_back(&vector, testobj_create(3));
        gvector_push_back(&vector, testobj_create(5));
        gvector_push_back(&vector, testobj_create(7));
        gvector_push_back(&vector, testobj_create(2));
        gvector_push_back(&vector, testobj_create(4));
        gvector_push_back(&vector, testobj_create(6));
        gvector_push_back(&vector, testobj_create(8));
        assert( !gvector_is_empty(&vector) );

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );
    }

    // Insert test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        size_t index;

        gvector_insert(&vector, index=0, testobj_create(8));
        /* 8 */

        gvector_insert(&vector, index=0, testobj_create(1));
        gvector_insert(&vector, index=1, testobj_create(3));
        /* 1, 3, 8 */

        index = gvector_get_count(&vector) - 1;  // Get last index.
        gvector_insert(&vector, index, testobj_create(6));
        /* 1, 3, 6, 8 */

        gvector_insert(&vector, index=2, testobj_create(5));
        gvector_insert(&vector, index=3, testobj_create(7));
        /* 1, 3, 5, 7, 6, 8 */

        index = gvector_get_count(&vector) - 1;  // Get last index.
        gvector_insert(&vector, --index, testobj_create(2));
        gvector_insert(&vector, ++index, testobj_create(4));
        /* 1, 3, 5, 7, 2, 4, 6, 8 */

        assert( gvector_get_count(&vector) == 8 );
        assert( verify_vector_values(&vector, target, 8) );
    }

    // Erase test
    {
        static const int target[] = { /*1,*/ 3, 5, /*7, 2,*/ 4, 6, /*8*/ };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        gvector_push_back(&vector, testobj_create(1));
        gvector_push_back(&vector, testobj_create(3));
        gvector_push_back(&vector, testobj_create(5));
        gvector_push_back(&vector, testobj_create(7));
        gvector_push_back(&vector, testobj_create(2));
        gvector_push_back(&vector, testobj_create(4));
        gvector_push_back(&vector, testobj_create(6));
        gvector_push_back(&vector, testobj_create(8));

        size_t index;

        gvector_erase(&vector, index=0);

        index = gvector_get_count(&vector) - 1;  // Get last index.
        gvector_erase(&vector, index);

        gvector_erase(&vector, index=2);
        gvector_erase(&vector, index=2);

        assert( gvector_get_count(&vector) == 4 );
        assert( verify_vector_values(&vector, target, 4) );

        int i;
        for(i=0; i<4; ++i)
        {
            gvector_erase(&vector, gvector_get_count(&vector)-1);
            gvector_erase(&vector, 0);
        }
        assert( gvector_get_count(&vector) == 0 );
        assert( verify_vector_values(&vector, NULL, 0) );
    }

    // Set and update item test
    {
        static const int target[] = { 1, 3, 5, 60, 2, 4, 6, 8 };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        gvector_set_item(&vector, 0, testobj_create(1));
        gvector_set_item(&vector, 1, testobj_create(3));
        gvector_set_item(&vector, 2, testobj_create(5));
        gvector_set_item(&vector, 3, testobj_create(7));
        gvector_set_item(&vector, 4, testobj_create(2));
        gvector_set_item(&vector, 5, testobj_create(4));
        gvector_set_item(&vector, 6, testobj_create(6));
        gvector_set_item(&vector, 7, testobj_create(8));

        gvector_set_item(&vector, 3, testobj_create(60));

        assert( gvector_get_count(&vector) == 8 );
        assert( verify_vector_values(&vector, target, 8) );

        gvector_clear(&vector);
        assert( testobj_refcnt == 0 );
    }

    // Container move test
    {
        static const int target[] = { 1, 3, 5, 7, 2, 4, 6, 8 };

        gvector_clear(&vector);
        assert( gvector_is_empty(&vector) );
        assert( testobj_refcnt == 0 );

        gvector_push_back(&vector, testobj_create(5));
        gvector_push_back(&vector, testobj_create(5));
        gvector_push_back(&vector, testobj_create(5));

        {
            gvector_t vector2;
            gvector_init(&vector2, (gvector_itemfree_t)testobj_release);

            gvector_push_back(&vector2, testobj_create(1));
            gvector_push_back(&vector2, testobj_create(3));
            gvector_push_back(&vector2, testobj_create(5));
            gvector_push_back(&vector2, testobj_create(7));
            gvector_push_back(&vector2, testobj_create(2));
            gvector_push_back(&vector2, testobj_create(4));
            gvector_push_back(&vector2, testobj_create(6));
            gvector_push_back(&vector2, testobj_create(8));

            gvector_movefrom(&vector, &vector2);

            assert( gvector_get_count(&vector2) == 0 );
            assert( verify_vector_values(&vector2, NULL, 0) );

            gvector_deinit(&vector2);
        }

        assert( gvector_get_count(&vector) == 8 );
        assert( verify_vector_values(&vector, target, 8) );

        gvector_clear(&vector);
        assert( testobj_refcnt == 0 );
    }

    gvector_deinit(&vector);
    assert( testobj_refcnt == 0 );

    return 0;
}

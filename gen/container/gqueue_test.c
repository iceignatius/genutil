#include <assert.h>
#include <stdlib.h>

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#include "./queue.h"

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

int main(int argc, char *argv[])
{
    // Push and pop test
    {
        testobj_t *item;

        gqueue_t queue;
        gqueue_init(&queue, (gqueue_itemfree_t)testobj_release);
        assert( 0 == gqueue_get_count(&queue) );

        // Push - part 1

        gqueue_push(&queue, testobj_create(1));
        assert( 1 == gqueue_get_count(&queue) );

        gqueue_push(&queue, testobj_create(2));
        assert( 2 == gqueue_get_count(&queue) );

        gqueue_push(&queue, testobj_create(3));
        assert( 3 == gqueue_get_count(&queue) );

        // Pop - part 1

        item = gqueue_get_first(&queue);
        assert( item );
        assert( 1 == item->value );
        gqueue_pop(&queue);
        assert( 2 == gqueue_get_count(&queue) );

        item = gqueue_get_first(&queue);
        assert( item );
        assert( 2 == item->value );
        gqueue_pop(&queue);
        assert( 1 == gqueue_get_count(&queue) );

        item = gqueue_get_first(&queue);
        assert( item );
        assert( 3 == item->value );
        gqueue_pop(&queue);
        assert( 0 == gqueue_get_count(&queue) );

        item = gqueue_get_first(&queue);
        assert( !item );
        gqueue_pop(&queue);
        assert( 0 == gqueue_get_count(&queue) );

        assert( testobj_refcnt == 0 );

        // Push - part 2

        gqueue_push(&queue, testobj_create(4));
        assert( 1 == gqueue_get_count(&queue) );

        // Pop - part 2

        item = gqueue_get_first(&queue);
        assert( item );
        assert( 4 == item->value );
        gqueue_pop(&queue);
        assert( 0 == gqueue_get_count(&queue) );

        item = gqueue_get_first(&queue);
        assert( !item );
        gqueue_pop(&queue);
        assert( 0 == gqueue_get_count(&queue) );

        assert( testobj_refcnt == 0 );

        gqueue_deinit(&queue);
        assert( testobj_refcnt == 0 );
    }

    // Clear test
    {
        gqueue_t queue;
        gqueue_init(&queue, (gqueue_itemfree_t)testobj_release);
        assert( 0 == gqueue_get_count(&queue) );

        gqueue_push(&queue, testobj_create(1));
        gqueue_push(&queue, testobj_create(2));
        gqueue_push(&queue, testobj_create(3));
        gqueue_push(&queue, testobj_create(4));
        assert( 4 == gqueue_get_count(&queue) );

        gqueue_clear(&queue);
        assert( 0 == gqueue_get_count(&queue) );
        assert( testobj_refcnt == 0 );

        gqueue_deinit(&queue);
        assert( testobj_refcnt == 0 );
    }

    return 0;
}

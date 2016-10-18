#include <assert.h>
#include <stdlib.h>
#include "stack.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

#include "stack.h"

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
    gstack_t stack;
    gstack_init(&stack, (gstack_itemfree_t)testobj_release);

    assert( gstack_get_count(&stack) == 0 );
    assert( testobj_refcnt == 0 );

    static const int tags[] = { 1, 3, 5, 7, 2, 4, 6, 8 };
    static const int tagcnt = sizeof(tags)/sizeof(tags[0]);

    // Push test.
    {
        assert( 0 == gstack_get_count(&stack) );
        assert( !gstack_get_top(&stack) );

        for(int i=0; i<tagcnt; ++i)
        {
            gstack_push(&stack, testobj_create(tags[i]));

            testobj_t *item = gstack_get_top(&stack);
            assert( item && item->value == tags[i] );
        }

        assert( tagcnt == gstack_get_count(&stack) );
    }

    // Pop test.
    {
        for(int i=tagcnt-1; i>=0; --i)
        {
            testobj_t *item = gstack_get_top(&stack);
            assert( item && item->value == tags[i] );

            gstack_pop(&stack);
        }

        assert( !gstack_get_top(&stack) );
        assert( 0 == gstack_get_count(&stack) );
    }

    // Clear test.
    {
        for(int i=0; i<tagcnt; ++i)
        {
            gstack_push(&stack, testobj_create(tags[i]));

            testobj_t *item = gstack_get_top(&stack);
            assert( item && item->value == tags[i] );
        }

        assert( tagcnt == gstack_get_count(&stack) );

        gstack_clear(&stack);
        assert( 0 == gstack_get_count(&stack) );
    }

    gstack_deinit(&stack);
    assert( testobj_refcnt == 0 );

    return 0;
}

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "inhab.h"

// Type 1, derived from inhab_t

typedef struct type1_t
{
    inhab_t inheritance;
} type1_t;

INHAB_CLASS_DECLARE(type1_t);
INHAB_CLASS_IMPLEMENT(type1_t)
#define TYPE1(instance) INHAB_CAST(instance, type1_t)

void type1_init(type1_t *obj)
{
    inhab_init(&obj->inheritance);
    INHAB_REPLACE_CLASS_INFO(obj, type1_t);
}

// Type 2, derived from type 1

typedef struct type2_t
{
    type1_t inheritance;
} type2_t;

INHAB_CLASS_DECLARE(type2_t);
INHAB_CLASS_IMPLEMENT(type2_t)
#define TYPE2(instance) INHAB_CAST(instance, type2_t)

void type2_init(type2_t *obj)
{
    type1_init(&obj->inheritance);
    INHAB_REPLACE_CLASS_INFO(obj, type2_t);
}

// Type 2, derived from type 2

typedef struct type3_t
{
    type2_t inheritance;
} type3_t;

INHAB_CLASS_DECLARE(type3_t);
INHAB_CLASS_IMPLEMENT(type3_t)
#define TYPE3(instance) INHAB_CAST(instance, type3_t)

void type3_init(type3_t *obj)
{
    type2_init(&obj->inheritance);
    INHAB_REPLACE_CLASS_INFO(obj, type3_t);
}

int main(void)
{
    type1_t obj1;
    type2_t obj2;
    type3_t obj3;

    type1_init(&obj1);
    type2_init(&obj2);
    type3_init(&obj3);

    printf("==== Cast test start. ====\n\n");

    INHAB(NULL);
    TYPE1(NULL);
    TYPE2(NULL);
    TYPE3(NULL);

    INHAB(&obj1);
    INHAB(&obj2);
    INHAB(&obj3);

    TYPE1(&obj1);
    TYPE2(&obj2);
    TYPE3(&obj3);

    TYPE2(&obj2);
    TYPE2(&obj3);

    TYPE3(&obj3);

    printf("\n==== There should not have any warning messages above. ====\n\n");

    TYPE2(&obj1);
    TYPE3(&obj1);
    TYPE3(&obj2);

    printf("\n==== There should have %d warning messages above. ====\n\n", 3);

    return 0;
}

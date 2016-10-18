#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "inhab.h"

INHAB_CLASS_IMPLEMENT(inhab_t)

//------------------------------------------------------------------------------
static
bool is_class_available(const inhab_class_t *class_info)
{
    return class_info && class_info->signature == INHAB_SIGNATURE;
}
//------------------------------------------------------------------------------
static
bool is_instance_available(const inhab_t *instance)
{
    return instance && is_class_available(instance->class_info);
}
//------------------------------------------------------------------------------
inhab_t* inhab_cast_instance_and_verify(inhab_t *instance, const inhab_class_t *target,
                                                           const char          *codefile,
                                                           int                  codeline)
{
#ifndef NDEBUG
    if( !instance || !target )
    {
        return instance;
    }

    if( !is_class_available(target) )
    {
        fprintf(stderr,
                "WARNING : Cast to unknown target type, file : %s, line : %d\n",
                codefile,
                codeline);
        return instance;
    }

    if( !is_instance_available(instance) )
    {
        fprintf(stderr,
                "WARNING : Invalid instance, file : %s, line : %d\n",
                codefile,
                codeline);
        return instance;
    }

    const inhab_class_t *class_info = instance->class_info;
    while( class_info )
    {
        if( !is_class_available(class_info) )
            break;

        if( class_info == target )
            return instance;

        class_info = class_info->parent;
    }

    fprintf(stderr,
            "WARNING : Invalid cast from \"%s\" to \"%s\", file : %s, line : %d\n",
            instance->class_info->class_name,
            target->class_name,
            codefile,
            codeline);
    return instance;
#else
    return instance;
#endif
}
//------------------------------------------------------------------------------
void inhab_init(inhab_t *object)
{
    assert( object );
    object->class_info = INHAB_CLASS_GET_INFO(inhab_t);
}
//------------------------------------------------------------------------------
void inhab_replace_class_info(inhab_t *instance, inhab_class_t *class_info,
                                                 const char    *codefile,
                                                 int            codeline)
{
#ifndef NDEBUG
    if( !is_class_available(class_info) )
    {
        fprintf(stderr,
                "ERROR : Try to set an invalid class info., file : %s, line : %d\n",
                codefile,
                codeline);
        exit(EXIT_FAILURE);
    }
#endif

#ifndef NDEBUG
    if( !is_instance_available(instance) )
    {
        fprintf(stderr,
                "ERROR : Invalid instance, file : %s, line : %d\n",
                codefile,
                codeline);
        exit(EXIT_FAILURE);
    }
#endif

#ifndef NDEBUG
    if( class_info == instance->class_info )
    {
        fprintf(stderr,
                "ERROR : Duplicate class replace, file : %s, line : %d\n",
                codefile,
                codeline);
        exit(EXIT_FAILURE);
    }
#endif

    if( !class_info->parent ) class_info->parent = instance->class_info;

#ifndef NDEBUG
    if( class_info->parent != instance->class_info )
    {
        fprintf(stderr,
                "ERROR : Conflict class parent, file : %s, line : %d\n",
                codefile,
                codeline);
        exit(EXIT_FAILURE);
    }
#endif

    instance->class_info = class_info;
}
//------------------------------------------------------------------------------

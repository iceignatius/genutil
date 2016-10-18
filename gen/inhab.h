/**
 * @file
 * @brief     Inheritable object (RTTI for C).
 * @details   Provide an inheritable base object class and tools to help user to
 *            build classes easily.
 * @author    王文佑
 * @date      2014.09.02
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_INHAB_H_
#define _GEN_INHAB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
    @file
    @verbatim
    Example:

    // We define our custom class (mytype_t) here:
    typedef struct mytype_t
    {
        // Inherit class, and it must be:
        // 1. The first member of this class.
        // 2. Derived from inhab_t.
        parent_t inheritance;

        // ... other members ...
    } mytype_t;

    // Build the implements needed for the class identifier by a macro INHAB_CLASS_IMPLEMENT, and:
    // 1. This statement MUST be UNIQUE for a class, we suggest to put it to the class source (*.c) file.
    // 2. Do not append semicolon after this statement.
    INHAB_CLASS_IMPLEMENT(mytype_t)

    // Here we define the type cast macro for our custom class:
    #define MYTYPE(instance) INHAB_CAST(instance,mytype_t)

    // But the type cast macro will need something from the INHAB_CLASS_IMPLEMENT,
    // so that we may need to declare something to avoid the compiler warnings:
    INHAB_CLASS_DECLARE(mytype_t);

    // Finally, we will need to add some operation to our class constructor:
    void mytype_init(mytype_t *self)
    {
        // Initialize the parent class, we may already done this:
        parent_init(&self->inheritance);
        // Then we need to replace the class information by our custom class information:
        INHAB_REPLACE_CLASS_INFO(self, mytype_t);

        // Please note that,
        // the parent class must be initialized before we replace the class information to our own.

        // ... the other construct operations ...
    }

    // The work now finished, this is all things we need to modify our class.
    // Now, the instance of other classes which derived form our class can be casted to our class type easily:
    MYTYPE(an_object_instance)

    @endverbatim
 */

typedef struct inhab_t inhab_t;

// Class Information
typedef struct inhab_class_t
{
    #define INHAB_SIGNATURE 0xE717103C
    unsigned              signature;
    struct inhab_class_t *parent;
    const char           *class_name;
} inhab_class_t;

// Class information operations

/// Class identifier declaration.
#define INHAB_CLASS_DECLARE(type)                        \
        inhab_class_t* type##_get_inhab_class_info(void)

/// Class identifier implementation.
#define INHAB_CLASS_IMPLEMENT(type)                                                            \
        static inhab_class_t type##_inhab_class_info = { INHAB_SIGNATURE, 0, #type };          \
        inhab_class_t* type##_get_inhab_class_info(void) { return & type##_inhab_class_info; }

#define INHAB_CLASS_GET_INFO(type) type##_get_inhab_class_info()

// Instance cast and verify

inhab_t* inhab_cast_instance_and_verify(inhab_t *instance, const inhab_class_t *target,
                                                           const char          *codefile,
                                                           int                  codeline);
#define INHAB_CAST_INSTANCE_AND_VERIFY(instance,target_class,target_type)   \
        (target_type*) inhab_cast_instance_and_verify((inhab_t*)(instance), \
                                                      target_class,         \
                                                      __FILE__,             \
                                                      __LINE__)

/// Cast to custom class type.
#ifdef NDEBUG
    #define INHAB_CAST(instance,target_type)                                  \
            (target_type*)(instance)
#else
    #define INHAB_CAST(instance,target_type)                                  \
            INHAB_CAST_INSTANCE_AND_VERIFY(instance,                          \
                                           INHAB_CLASS_GET_INFO(target_type), \
                                           target_type)
#endif

/// Base class.
struct inhab_t
{
    inhab_class_t *class_info;
};

INHAB_CLASS_DECLARE(inhab_t);
#define INHAB(instance) INHAB_CAST(instance,inhab_t)

void inhab_init(inhab_t *object);

void inhab_replace_class_info(inhab_t *instance, inhab_class_t *class_info,
                                                 const char    *codefile,
                                                 int            codeline);

/// Initialize class information when construct.
#define INHAB_REPLACE_CLASS_INFO(instance,type)              \
        inhab_replace_class_info((inhab_t*)(instance),       \
                                 INHAB_CLASS_GET_INFO(type), \
                                 __FILE__,                   \
                                 __LINE__)

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

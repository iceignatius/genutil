/**
 * @file
 * @brief     Jump block
 * @details   To implement the local try-catch mechanism for C code.
 * @author    王文佑
 * @date      2014.10.16
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_JMPBK_H_
#define _GEN_JMPBK_H_

/**
    @file
    @par Example
    @verbatim

    // Prepare to catch any error codes.
    // And note that the curly bracket pare are not necessary for all JMPBK statements,
    // but we recommended to add them for a better code view.
    JMPBK_BEGIN
    {
        ...
        ...
        if( ... ) JMPBK_THROW(7);  // Throw an error code here, the code can be any value except ZERO.
        ...
        if( ... ) JMPBK_THROW(0);  // Break and go to error catch block.
                                   // Throw ZERO value means that you do not care what the final error code is.
        ...
        ...  // The statements here will be skipped if we has throw an error code above.
        ...  // Then the flow will jump to JMPBLK_END, or one of the catch blocks if it has.
        ...  // And note that the flow will jump to JMPBK_FINAL block after the catch blocks
        ...  // if there has a final block.
        ...
        // We can use JMPBK_ERRCODE to get the error code,
        // although the error code will be ZERO if no any code has been thrown.
        printf("Error Code : %d\n", JMPBK_ERRCODE);
        ...
        ...
        ...  // When the code block ended normally,
        ...  // the execution flow will jump to JMPBLK_END or JMPBK_FINAL if it have,
        ...  // if there has no any throw statement be executed.
    }
    // All error code catch blocks are optional.
    JMPBK_CATCH(3)
    {
        ...  // The execution flow will come here if an error code 3 had been thrown.
        ...
    }
    // Catch for another error code.
    JMPBK_CATCH(7)
    {
        ...
        // It is not allowed to throw error code here,
        // the throw can be used before any catch or final statements only.
        JMPBK_THROW(7);
    }
    // Remember that, the error code ZERO is NOT ACCEPTABLE.
    JMPBK_CATCH(0)
    {
        ...  // You can never catch the zero code.
        ...
    }
    // The catch all block is optional, too.
    JMPBK_CATCH_ALL
    {
        ...  // Execution flow will come here if an error code thrown but not be catch above.
        ...
        printf("Error Code : %d\n", JMPBK_ERRCODE);  // We can use JMPBK_ERRCODE to get the error code.
        ...
    }
    // The final block is optional, too.
    JMPBK_FINAL
    {
        ...  // Execution flow will finally come here no matter if an error code has been thrown.
        ...
        printf("Error Code : %d\n", JMPBK_ERRCODE);
        ...
    }
    // Finally, we need to use JMPBK_END to end anything of the jump blocks,
    // and this is mandatory.
    JMPBK_END

    // Execution flow will come here after all JMPBK process finished.

    @endverbatim
 */
/**
 * @file
 * @attention The JMPBK tool may have some limits:
 * @li It does not support nested usage.
 * @li It can not jump through function calls, and can be used in a single function.
 * @li It does not support duplicated JUMBK process in a function.
 */

/// Block begin.
#define JMPBK_BEGIN                                     \
        {                                               \
            int  __jmpbk_errcode = 0;                   \
        __jmpbk_begin:                                  \
            switch( __jmpbk_errcode )                   \
            {                                           \
                case 0:                                 \
                {

/// Catch specific error code.
#define JMPBK_CATCH(errcode)                            \
                }                                       \
                break;                                  \
                case (errcode):                         \
                {

/// Catch all error codes.
#define JMPBK_CATCH_ALL                                 \
                }                                       \
                break;                                  \
                default:                                \
                {

/// Final process.
#define JMPBK_FINAL                                     \
                }                                       \
            }                                           \
            {                                           \
                {

/// Block end.
#define JMPBK_END                                       \
                }                                       \
            }                                           \
        }

/// Throw an error code.
#define JMPBK_THROW(errcode)                            \
        do{                                             \
            __jmpbk_errcode = errcode ? errcode : -1;   \
            goto __jmpbk_begin;                         \
        } while(0)

/// The error code thrown by @ref JMPBK_THROW.
#define JMPBK_ERRCODE  __jmpbk_errcode

#endif

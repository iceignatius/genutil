/**
 * @file
 * @brief     Shared Library Definition.
 * @details   General definition of shared library.
 * @author    王文佑
 * @date      2014.02.11
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_SHRDLIBDEF_H_
#define _GEN_SHRDLIBDEF_H_

/**
 * @file
 * 這個巨集定義檔是專為製做動態的共享程式庫所設計，其定義之巨集專用來修飾程式庫
 * 的導出函式，使一個通用的跨平臺程式庫程式碼能容易的被建立。在使用本定義檔建立
 * 程式庫時，有以下幾點事項建議使用者：
 * @li 製做程式庫的專案應定義一個屬於自己的巨集標籤做為識別。
 * @li 依據自訂巨集標籤定義程式庫的導入函式與導出函式修飾巨集，並以此巨集修飾所有
 *     程式庫的導出函式，用法範例可見下方描述。
 * @li 在 Linux 上以 GCC 建置共享程式庫時建議加入下列編譯選項，以關閉所非導出函式
 *     的可見度：
 *     -fvisibility=hidden
 * @li 本定義集在目前只支援 Linux 與 Windows 兩作業平臺。在 Linux 上只支援 GCC 編
 *     譯器，使用其他編譯器的效果不能被保證。
 *
 * @par 巨集說明
 *      @li SHRDLIB_EXTC   : 用來指定一個函式將使用 C 語言命名規則來編譯，
 *                           一般將此修飾加在函式宣告處的最前面。
 *      @li SHRDLIB_IMPORT : 設定函式為導入函式，一般將此修飾加在函式宣告的最前面、
 *                           SHRDLIB_EXTC 的後面。
 *      @li SHRDLIB_EXPORT : 設定函式為導出函式，
 *      @li SHRDLIB_CALL   : 指定函式的呼叫慣例，此修飾請加在函式傳回值與函式名之間，
 *                           函式宣告與實做處都需以此處理。
 */
/**
    @file
    @par 使用範例
    @verbatim

    [ *.h ]

    // MAKESHARED 假設為製做程式庫的專案所定義的巨集，
    // 而一般使用端不會有這個巨集定義。
    #ifdef MAKESHARED
        #MODFUNC(type) SHRDLIB_EXTC SHRDLIB_EXPORT type SHRDLIB_CALL
    #else
        #MODFUNC(type) SHRDLIB_EXTC SHRDLIB_IMPORT type SHRDLIB_CALL
    #endif

    MODFUNC(int) foo(int x);

    [ *.c / *.cpp ]

    int SHRDLIB_CALL foo(int x){ return x; }

    @endverbatim
 */

// For document use only
#ifdef DOXYGEN
    #define SHRDLIB_EXTC    ///< Extern in "C" format.
    #define SHRDLIB_IMPORT  ///< Identify an import function.
    #define SHRDLIB_EXPORT  ///< Identify an export function.
    #define SHRDLIB_CALL    ///< Calling convention.
#endif

// Extern in "C" format
#ifdef __cplusplus
    #define  SHRDLIB_EXTC  extern "C"
#else
    #define  SHRDLIB_EXTC
#endif

// Function import and export
#if   defined(__linux__) && defined(__GNUC__)
    #define  SHRDLIB_IMPORT
    #define  SHRDLIB_EXPORT  __attribute__((visibility("default")))
#elif defined(_WIN32)
    #define  SHRDLIB_IMPORT  __declspec(dllimport)
    #define  SHRDLIB_EXPORT  __declspec(dllexport)
#else
    #error No implementation on this platform!
#endif

// Calling convention
#if   defined(__linux__) && defined(__i386__) && defined(__GNUC__)
    #define  SHRDLIB_CALL  __attribute__((cdecl))
#elif defined(_WIN32)
    #define  SHRDLIB_CALL  __stdcall
#else
    #define  SHRDLIB_CALL
#endif

#endif

/**
 * @file
 * @brief     Windows WSA.
 * @details   Encapsulation of Windows WSA.
 * @author    王文佑
 * @date      2014.03.12
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
/**
 * @file
 * 對於 C 語言的使用者，本模組已將 Windows WSA 的初始化相關函式封裝為可遞迴呼叫，
 * 因此只要記得多次的 Initialize 呼叫要搭配相同次數的 De-initialize 函數即可。
 */
/**
 * @file
 * @par Windows WSA 封裝物件(C++)
 *
 * 本模組以獨體的形式封裝 Windows WSA 的操作，所有的主要功能都在建構子和解構子完成，
 * 使用者一般來說只需要在欲使用網路功能的函式裡建立本物件即可，請見以下範例：
 */
/**
    @file
    @verbatim

    void UserFunction()
    {
        TWinWsa WSA;                     // 只要這樣子就可以了，建構子和解構子會做相關的 WSA 初始與結束程序。

        assert( WSA.IsAvailable() );     // 若想要檢查初始化是否成功的話可以呼叫這個函式。
        assert( WSA.StartExplicit() );   // 也可以顯式的呼叫初始化函式，如果 WSA 從前沒有成功啟動的話，也可以用這個函式在程式執行中重新啟動。

        // Network operating ...
        AnotherUserFunction();
        // ...
    }

    void AnotherUserFunction()
    {
        TWinWsa WSA;                     // 由於本物件是以獨體的形式封裝，因此使用者只管在需要的地方建立物件就可以了！

        // Network operating ...
        // ...
    }

    @endverbatim
 */
#ifndef _GEN_NET_WINWSA_H_
#define _GEN_NET_WINWSA_H_

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

bool        winwsa_init_recursive      (void);
void        winwsa_deinit_recursive    (void);
bool        winwsa_start_explicit      (void);
bool        winwsa_is_available        (void);
int         winwsa_get_start_error_code(void);
int         winwsa_get_last_error_code (void);
const char* winwsa_get_error_message   (int errcode);
void        winwsa_set_last_error_code (int errcode);

#else

static inline bool        winwsa_init_recursive      (void)        { return true; }
static inline void        winwsa_deinit_recursive    (void)        {}
static inline bool        winwsa_start_explicit      (void)        { return true; }
static inline bool        winwsa_is_available        (void)        { return true; }
static inline int         winwsa_get_start_error_code(void)        { return 0; }
static inline int         winwsa_get_last_error_code (void)        { return 0; }
static inline const char* winwsa_get_error_message   (int errcode) { return ""; }
static inline void        winwsa_set_last_error_code (int errcode) {}

#endif  // _WIN32

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of Windows WSA object.
class TWinWsa
{
public:
    TWinWsa ()                  { winwsa_init_recursive(); }
    TWinWsa (const TWinWsa &Src){ winwsa_init_recursive(); }
    ~TWinWsa()                  { winwsa_deinit_recursive(); }
public:
    TWinWsa& operator=(const TWinWsa &Src) { return *this; }

public:
    static bool        StartExplicit    ()            { return winwsa_start_explicit(); }              ///< @see ::winwsa_start_explicit
    static bool        IsAvailable      ()            { return winwsa_is_available(); }                ///< @see ::winwsa_is_available
    static int         GetStartErrorCode()            { return winwsa_get_start_error_code(); }        ///< @see ::winwsa_get_start_error_code
    static int         GetLastErrorCode ()            { return winwsa_get_last_error_code(); }         ///< @see ::winwsa_get_last_error_code
    static std::string GetErrorMessage  (int errcode) { return winwsa_get_error_message(errcode); }    ///< @see ::winwsa_get_error_message
    static void        SetLastErrorCode (int errcode) {        winwsa_set_last_error_code(errcode); }  ///< @see ::winwsa_set_last_error_code


};

#endif

#endif

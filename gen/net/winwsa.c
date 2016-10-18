#ifdef _WIN32
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <winsock2.h>
#include "../utf.h"
#include "winwsa.h"

#ifdef _MSC_VER
    #define snprintf _snprintf
#endif

#ifndef _tcslen
    #ifdef UNICODE
        #define _tcslen wcslen
    #else
        #define _tcslen strlen
    #endif
#endif

static atomic_flag      wsalock_inited = ATOMIC_FLAG_INIT;
static CRITICAL_SECTION wsalock;

static atomic_int start_errcode = ATOMIC_VAR_INIT(WSASYSNOTREADY);
static atomic_int ref_count     = ATOMIC_VAR_INIT(0);

//------------------------------------------------------------------------------
static
void wsalock_init_if_not_inited(void)
{
    if( !atomic_flag_test_and_set(&wsalock_inited) )
        InitializeCriticalSection(&wsalock);
}
//------------------------------------------------------------------------------
static
int wsa_startup(void)
{
    int errcode;

    wsalock_init_if_not_inited();
    EnterCriticalSection(&wsalock);
    {
        static const WORD wsa_version = MAKEWORD(2,2);
        WSADATA           wsa_data;

        errcode = WSAStartup(wsa_version, &wsa_data);
        atomic_store(&start_errcode, errcode);
    }
    LeaveCriticalSection(&wsalock);

    return errcode;
}
//------------------------------------------------------------------------------
static
void wsa_cleanup(void)
{
    wsalock_init_if_not_inited();
    EnterCriticalSection(&wsalock);
    {
        WSACleanup();
        atomic_store(&start_errcode, WSASYSNOTREADY);
    }
    LeaveCriticalSection(&wsalock);
}
//------------------------------------------------------------------------------
static
bool ref_increase(void)
{
    int old_count = atomic_fetch_add(&ref_count, 1);
    return !old_count;
}
//------------------------------------------------------------------------------
static
bool ref_decrease(void)
{
    int old_count = atomic_fetch_sub(&ref_count, 1);
    assert( old_count );
    return !atomic_load(&ref_count);
}
//------------------------------------------------------------------------------
bool winwsa_init_recursive(void)
{
    /**
     * Initialize Windows WSA.
     *
     * @return TRUE if succeed; and FALSE if failed.
     *
     * @remarks This function can be called many times,
     *          and will need to call ::winwsa_deinit_recursive for the same times
     *          to finish Windows WSA.
     */
    return ( ref_increase() || atomic_load(&start_errcode) )?
           ( wsa_startup() ):( true );
}
//------------------------------------------------------------------------------
void winwsa_deinit_recursive(void)
{
    /**
     * Finish Windows WSA.
     */
    if( ref_decrease() ) wsa_cleanup();
}
//------------------------------------------------------------------------------
bool winwsa_start_explicit(void)
{
    /**
     * 顯式的啟動 Windows WSA。
     * @return 傳回啟動是否成功的邏輯。
     */
    if( !atomic_load(&ref_count) ) return false;
    if( !atomic_load(&start_errcode) ) return true;

    return !wsa_startup();
}
//------------------------------------------------------------------------------
bool winwsa_is_available(void)
{
    /**
     * 檢查 Windows WSA 是否已啟用。
     * @return 傳回 Windows WSA 是否已啟用的邏輯。
     */
    return !atomic_load(&start_errcode);
}
//------------------------------------------------------------------------------
int winwsa_get_start_error_code(void)
{
    /**
     * 取得 Windows WSA 啟動時的返回碼/錯誤碼。
     * @return Windows WSA 啟動時的返回碼/錯誤碼。
     */
    return atomic_load(&start_errcode);
}
//------------------------------------------------------------------------------
int winwsa_get_last_error_code(void)
{
    /**
     * 取得最後一次操作 Windows Socket 功能的返回碼/錯誤碼。
     * @return 最後一次操作 Windows Socket 功能的返回碼/錯誤碼。
     */
    return WSAGetLastError();
}
//------------------------------------------------------------------------------
const char* winwsa_get_error_message(int errcode)
{
    /**
     * 查詢 Windows API 錯誤碼所對應的文字訊息。
     *
     * @param errcode Windows API 錯誤碼。
     * @return 錯誤碼所對應的文字訊息。
     */
    static TCHAR errmsg_winstr[0x0FFF] = {0};
    static char  errmsg_utf8  [0x0FFF] = {0};

    // 將代碼查詢轉換為文字訊息
    if( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      errcode,
                      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Default language
                      errmsg_winstr,
                      sizeof(errmsg_winstr)/sizeof(errmsg_winstr[0]),
                      NULL) )
    {
        size_t utf8_conv_result;

        // 去除換行字元
        if( _tcslen(errmsg_winstr) > 2 ) errmsg_winstr[_tcslen(errmsg_winstr)-2] = 0;

        utf8_conv_result = winstr_to_utf8(errmsg_utf8, sizeof(errmsg_utf8)-1, errmsg_winstr);
        assert( utf8_conv_result );
    }
    else
    {
        snprintf(errmsg_utf8,
                 sizeof(errmsg_utf8)-1,
                 "Calling function \"FormatMessage\" failed with error code 0x%X",
                 (unsigned int)GetLastError());
    }

    return errmsg_utf8;
}
//------------------------------------------------------------------------------
void winwsa_set_last_error_code(int errcode)
{
    /**
     * 設定 Windows Socket 的操作錯誤碼。
     * @param errcode 欲設定的錯誤碼。
     */
    WSASetLastError(errcode);
}
//------------------------------------------------------------------------------

#endif  // _WIN32

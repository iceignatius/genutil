/**
 * @file
 * @brief   Windows DLL entrance.
 * @details This file is the implementation about a Windows DLL entrance,
 *          and just add the whole file to your project if you need a DLL entrance
 *          and you do not have one.
 */
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef DOXYGEN
    /// This file implemented a function of DLL entrance,
    /// and the whole file will have no effect if not in Windows.
    BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
#endif

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        // Attach to process,
        // return FALSE to DLL load failed.
        break;

    case DLL_PROCESS_DETACH:
        // Detach from process.
        break;

    case DLL_THREAD_ATTACH:
        // Attach to thread.
        break;

    case DLL_THREAD_DETACH:
        // Detach from thread.
        break;

    }

    return TRUE; // succesful
}
#endif

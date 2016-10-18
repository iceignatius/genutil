#ifdef __linux__
#include <dlfcn.h>  // Also remember to link "dl" library (-ldl)
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "utf.h"
#include "shrdlib.h"

//------------------------------------------------------------------------------
shrdlib_t shrdlib_open(const char* RESTRICT filename)
{
    /**
     * Load a dynamic shared library.
     * @param filename File name of the dynamic shared library.
     * @return A library handle if succeed; and NULL if failed.
     */
#if   defined(__linux__)
    return dlopen(filename, RTLD_LAZY);
#elif defined(_WIN32)
    TCHAR winname[260];

    if( !utf8_to_winchar(winname, sizeof(winname), filename) )
        return NULL;

    return LoadLibrary(winname);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool shrdlib_close(shrdlib_t* RESTRICT handle)
{
    /**
     * Unload a dynamic shared library.
     * @param handle A handler of the loaded library.
     * @return TRUE if succeed; and FALSE if failed.
     */
#if   defined(__linux__)
    return !dlclose(handle);
#elif defined(_WIN32)
    return FreeLibrary((HMODULE)handle);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void* shrdlib_getfunc(shrdlib_t* RESTRICT handle, const char* RESTRICT funcname)
{
    /**
     * Get the function address of the library.
     * @param handle   A handler of the loaded library.
     * @param funcname Name of the function exported form the library.
     * @return Address of the symbol; and NULL if failed.
     */
#if   defined(__linux__)
    return dlsym(handle, funcname);
#elif defined(_WIN32)
    return GetProcAddress((HMODULE)handle, funcname);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------

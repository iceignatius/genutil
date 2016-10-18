#ifdef _WIN32

#if defined(UNICODE) && !defined(_UNICODE)
    // Define Unicode macro for MinGW.
    #define _UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "utf.h"
#include "windirent.h"

struct DIR
{
    HANDLE          hfind;
    WIN32_FIND_DATA finddata;
    struct dirent   dirent;
};

//------------------------------------------------------------------------------
DIR* opendir(const char *name)
{
    /**
     * Start directory enumeration.
     * Please refer to the official document of POSIX APIs.
     */
    DIR *dir = NULL;

    bool res = false;
    do
    {
        TCHAR tname[MAX_PATH+1] = {0};
        utf8_to_winstr(tname, MAX_PATH, name);

        // Append search mask.
        if( tname[_tcslen(tname)-1] != '/' || tname[_tcslen(tname)-1] != '\\' )
            _tcsncat(tname, _T("/"), MAX_PATH - _tcslen(tname));
        _tcsncat(tname, _T("*"), MAX_PATH - _tcslen(tname));

        dir = malloc(sizeof(DIR));
        if( !dir ) break;
        memset(dir, 0, sizeof(*dir));

        dir->hfind = FindFirstFile(tname, &dir->finddata);
        if( dir->hfind == INVALID_HANDLE_VALUE ) break;

        res = true;
    } while(false);

    if( !res )
    {
        closedir(dir);
        dir = NULL;
    }

    return dir;
}
//------------------------------------------------------------------------------
int closedir(DIR *dir)
{
    /**
     * Stop directory enumeration.
     * Please refer to the official document of POSIX APIs.
     */
    if( !dir ) return 0;

    int res = ( dir->hfind == INVALID_HANDLE_VALUE )?( 0 ):( FindClose(dir->hfind) ? 0 : -1 );
    free(dir);

    return res;
}
//------------------------------------------------------------------------------
static
unsigned char fileattr_to_filetype(DWORD attr)
{
    switch(attr)
    {
    case FILE_ATTRIBUTE_DIRECTORY:
        return DT_DIR;

    case FILE_ATTRIBUTE_REPARSE_POINT:
        return DT_LNK;

    case FILE_ATTRIBUTE_NORMAL:
        return DT_REG;

    default:
        return DT_UNKNOWN;

    }
}
//------------------------------------------------------------------------------
static
void finddata_to_dirent(struct dirent *info, WIN32_FIND_DATA *finddata)
{
    info->d_ino    = 0;
    info->d_off    = 0;
    info->d_reclen = sizeof(*info);
    info->d_type   = fileattr_to_filetype(finddata->dwFileAttributes);
    winstr_to_utf8(info->d_name, sizeof(info->d_name)-1, finddata->cFileName);
}
//------------------------------------------------------------------------------
struct dirent *readdir(DIR *dir)
{
    /**
     * Read one directory sub item.
     * Please refer to the official document of POSIX APIs.
     */
    if( !dir ) return NULL;

    if( dir->dirent.d_reclen )
    {
        // directory has been read before.
        if( !FindNextFile(dir->hfind, &dir->finddata) )
            return NULL;
    }

    finddata_to_dirent(&dir->dirent, &dir->finddata);
    return &dir->dirent;
}
//------------------------------------------------------------------------------

#endif // _WIN32

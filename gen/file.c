#if defined(UNICODE) && !defined(_UNICODE)
    // Define Unicode macro for MinGW.
    #define _UNICODE
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <sys/stat.h>
#include "utf.h"
#include "windirent.h"
#endif

#include "minmax.h"
#include "jmpbk.h"
#include "file.h"

#if   defined(__linux__)
    #define PATH_MAX_LEN PATH_MAX
#elif defined(_WIN32)
    #define PATH_MAX_LEN MAX_PATH
#else
    #define PATH_MAX_LEN 1024  // A size may large enough to be the default.
#endif

//------------------------------------------------------------------------------
//---- Null-separated string list ----------------------------------------------
//------------------------------------------------------------------------------
static
char* strlist_get_next(char *currpos)
{
    return currpos + strlen(currpos) + 1;
}
//------------------------------------------------------------------------------
static
char* strlist_get_last(char *strlist)
{
    char *pos  = strlist;
    char *next = strlist_get_next(pos);
    while( *next )
    {
        pos  = next;
        next = strlist_get_next(pos);
    }

    return pos;
}
//------------------------------------------------------------------------------
static
char* strlist_get_endpos(char *strlist)
{
    char *pos = strlist;
    if( !*pos ) return pos;

    for(; pos[0] || pos[1]; ++pos)
    {}

    return pos + 1;
}
//------------------------------------------------------------------------------
static
unsigned strlist_get_count(char *strlist)
{
    unsigned cnt = 0;

    char *pos;
    for(pos = strlist;
        *pos;
        pos = strlist_get_next(pos))
    {
        ++cnt;
    }

    return cnt;
}
//------------------------------------------------------------------------------
static
void strlist_append(char *strlist, size_t bufsz, const char *str, size_t maxappend)
{
    char *pos = strlist_get_endpos(strlist);
    size_t maxbufsz = bufsz - ( pos - strlist ) - 2;
    maxappend = MIN( maxappend, maxbufsz );
    strncpy(pos, str, maxappend);
}
//------------------------------------------------------------------------------
static
void strlist_remove_last(char *strlist)
{
    char *pos = strlist_get_last(strlist);
    *pos = 0;
}
//------------------------------------------------------------------------------
//---- Path style --------------------------------------------------------------
//------------------------------------------------------------------------------
void file_to_unix_path(char *path)
{
    /**
     * Convert path to UNIX style.
     *
     * @param[in,out] path Input a path string and output an UNIX style path string.
     */
    for(; *path; ++path)
    {
        if( *path == '\\' )
            *path = '/';
    }
}
//------------------------------------------------------------------------------
void file_to_windows_path(char *path)
{
    /**
     * Convert path to Windows style.
     *
     * @param[in,out] path Input a path string and output an Windows style path string.
     */
    for(; *path; ++path)
    {
        if( *path == '/' )
            *path = '\\';
    }
}
//------------------------------------------------------------------------------
//---- Properties inquiry ------------------------------------------------------
//------------------------------------------------------------------------------
size_t file_get_max_path_len(void)
{
    /**
     * Get the maximum path length that the local system can handle.
     */
    return PATH_MAX_LEN;
}
//------------------------------------------------------------------------------
//---- Path inquiry ------------------------------------------------------------
//------------------------------------------------------------------------------
const char* file_get_working_dir(void)
{
    /**
     * Get current working directory.
     *
     * @remarks The return value points to an internal static buffer,
     *          and what it contained may be modified when
     *          every time someone call this function.
     */
    static char pathname[PATH_MAX_LEN] = {0};

#if   defined(__linux__)
    if( !getcwd(pathname, sizeof(pathname)-1) )
        pathname[0] = 0;
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1];
    DWORD len = GetCurrentDirectory(PATH_MAX_LEN, tstr);
    if( !len || len > PATH_MAX_LEN )
        tstr[0] = 0;

    winstr_to_utf8(pathname, sizeof(pathname)-1, tstr);
    file_to_unix_path(pathname);
#else
    #error No implimentation on this platform!
#endif

    return pathname;
}
//------------------------------------------------------------------------------
const char* file_get_home_dir(void)
{
    /**
     * Get home directory for current user.
     *
     * @remarks The return value points to an internal static buffer,
     *          and what it contained may be modified when
     *          every time someone call this function.
     */
    static char pathname[PATH_MAX_LEN] = {0};

#if   defined(__linux__)
    struct passwd *pwd = getpwuid(getuid());
    strncpy(pathname, ( pwd ? pwd->pw_dir : "" ), sizeof(pathname)-1);
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1];
    if( FAILED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, tstr)) )
        tstr[0] = 0;

    winstr_to_utf8(pathname, sizeof(pathname)-1, tstr);
    file_to_unix_path(pathname);
#else
    #error No implimentation on this platform!
#endif

    return pathname;
}
//------------------------------------------------------------------------------
const char* file_get_temp_dir(void)
{
    /**
     * Get system temporary directory.
     *
     * @remarks The return value points to an internal static buffer,
     *          and what it contained may be modified when
     *          every time someone call this function.
     */
    static char pathname[PATH_MAX_LEN] = {0};

#if   defined(__linux__)
    strncpy(pathname, "/tmp", sizeof(pathname)-1);
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1];
    DWORD len = GetTempPath(PATH_MAX_LEN, tstr);
    if( !len || len > PATH_MAX_LEN )
        tstr[0] = 0;
    else
        tstr[len-1] = 0;  // Remove the last slash.

    winstr_to_utf8(pathname, sizeof(pathname)-1, tstr);
    file_to_unix_path(pathname);
#else
    #error No implimentation on this platform!
#endif

    return pathname;
}
//------------------------------------------------------------------------------
//---- Path name operation -----------------------------------------------------
//------------------------------------------------------------------------------
char* file_set_file_ext(char *buf, size_t bufsz, const char *file, const char *ext)
{
    /**
     * Get file name extension.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The old file name.
     * @param ext   The file name extension to set to the file name.
     * @return The output buffer.
     */
    const char *pos = strrchr(file, '.');
    if( !pos ) pos = file + strlen(file);

    size_t namelen = pos - file;

    memset(buf, 0, bufsz);
    strncpy(buf, file, namelen);
    if( bufsz < namelen ) return buf;

    strncat(buf, ext, bufsz-namelen);
    return buf;
}
//------------------------------------------------------------------------------
char* file_get_file_ext(char *buf, size_t bufsz, const char *file)
{
    /**
     * Get file name extension.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file name to inquiry.
     * @return The output buffer.
     */
    const char *pos = strrchr(file, '.');
    if( !pos ) pos = "";

    strncpy(buf, pos, bufsz);
    return buf;
}
//------------------------------------------------------------------------------
char* file_set_file_ext_all(char *buf, size_t bufsz, const char *file, const char *ext)
{
    /**
     * Get file name total extension.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The old file name.
     * @param ext   The file name extension to set to the file name.
     * @return The output buffer.
     */
    const char *namepos = strrchr(file, '/');
    if( !namepos ) namepos = file;

    const char *recpos = strchr(namepos, '.');
    if( !recpos ) recpos = namepos + strlen(namepos);

    size_t namelen = recpos - file;

    memset(buf, 0, bufsz);
    strncpy(buf, file, namelen);
    if( bufsz < namelen ) return buf;

    strncat(buf, ext, bufsz-namelen);
    return buf;
}
//------------------------------------------------------------------------------
char* file_get_file_ext_all(char *buf, size_t bufsz, const char *file)
{
    /**
     * Get file name total extension.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file name to inquiry.
     * @return The output buffer.
     */
    const char *namepos = strrchr(file, '/');
    if( !namepos ) namepos = file;

    const char *extpos = strchr(namepos, '.');
    if( !extpos ) extpos = "";

    strncpy(buf, extpos, bufsz);
    return buf;
}
//------------------------------------------------------------------------------
char* file_get_file_name(char *buf, size_t bufsz, const char *file)
{
    /**
     * Get file name from a path name string.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file path name to inquiry.
     * @return The output buffer.
     */
    char path[PATH_MAX_LEN] = {0};
    strncpy(path, file, sizeof(path)-1);

    file_to_unix_path(path);

    char *pos = strrchr(path, '/');
    pos = ( pos )?( pos + 1 ):( path );

    strncpy(buf, pos, bufsz);
    return buf;
}
//------------------------------------------------------------------------------
char* file_get_file_dir(char *buf, size_t bufsz, const char *file)
{
    /**
     * Get the previous directory of a path name string.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file path name to inquiry.
     * @return The output buffer.
     */
    char path[PATH_MAX_LEN] = {0};
    strncpy(path, file, sizeof(path)-1);

    file_to_unix_path(path);

    char *pos = strrchr(path, '/');
    if( pos )
        *pos = 0;
    else
        path[0] = 0;

    strncpy(buf, path, bufsz);
    return buf;
}
//------------------------------------------------------------------------------
static
bool is_root_path(const char *path)
{
    return ( path[0] == '/' && !path[1] ) ||
           ( path[2] == '/' && path[1] == ':' );
}
//------------------------------------------------------------------------------
char* file_get_file_path(char *buf, size_t bufsz, const char *file)
{
    /**
     * Get the previous path of a path name string.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file path name to inquiry.
     * @return The output buffer.
     */
    char path[PATH_MAX_LEN] = {0};
    strncpy(path, file, sizeof(path)-1);

    file_to_unix_path(path);

    while(true)
    {
        char *pos = strrchr(path, '/');
        if( !pos )
        {
            // No slash be found.
            // Remove all characters then.
            path[0] = 0;
        }
        else if( pos[1] )
        {
            // The slash is not the last character.
            // Remove characters after it then.
            pos[1] = 0;
        }
        else if( is_root_path(path) )
        {
            // The total path is a root path.
            // Save the total path and do not modify it then.
        }
        else
        {
            // Truncate the path and run this process again.
            *pos = 0;
            continue;
        }

        break;
    }

    strncpy(buf, path, bufsz);
    return buf;
}
//------------------------------------------------------------------------------
static
bool have_home_path(const char *str)
{
    return str[0] == '~' && ( str[1] == '/' || str[1] == '\\' );
}
//------------------------------------------------------------------------------
static
bool have_root_path(const char *str)
{
    return ( str[0] == '/' ) ||
           ( str[0] && str[1] == ':' && ( str[2] == '/' || str[2] == '\\' ) );
}
//------------------------------------------------------------------------------
static
void append_abs_path_prefix(char *dest, size_t destsz, const char *src)
{
    memset(dest, 0, destsz);
    if( !destsz || !src ) return;

    if( have_root_path(src) )
    {
        strncpy(dest, src, destsz-1);
    }
    else if( have_home_path(src) )
    {
        strncpy(dest, file_get_home_dir(), destsz-1);
        strncat(dest, src+1, destsz-strlen(dest)-1);  // Append source string without the home mark.
    }
    else
    {
        strncpy(dest, file_get_working_dir(), destsz-1);
        strncat(dest, "/", destsz-strlen(dest)-1);
        strncat(dest, src, destsz-strlen(dest)-1);
    }
}
//------------------------------------------------------------------------------
static
void path_str_to_strlist(char *listbuf, size_t bufsz, const char *path)
{
    memset(listbuf, 0, bufsz);

    const char *pos = path;
    const char *next;
    while(( next = strchr(pos, '/') ))
    {
        strlist_append(listbuf, bufsz, pos, next-pos+1);
        pos = next + 1;
    }

    strlist_append(listbuf, bufsz, pos, -1);
}
//------------------------------------------------------------------------------
static
void strlist_to_path_str(char *buf, size_t bufsz, const char *strlist)
{
    if( !bufsz ) return;

    memset(buf, 0, bufsz);
    --bufsz;

    char *pos;
    for(pos = (char*)strlist;
        *pos && bufsz;
        pos = strlist_get_next(pos))
    {
        strncpy(buf, pos, bufsz);
        size_t fillsz = strlen(buf);
        buf   += fillsz;
        bufsz -= fillsz;
    }
}
//------------------------------------------------------------------------------
char* file_get_abs_path(char *buf, size_t bufsz, const char *file)
{
    /**
     * Calculate absolute path of a file path name.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file path name to inquiry.
     * @return The output buffer.
     */
    if( !bufsz ) return buf;

    if( !file[0] )
    {
        buf[0] = 0;
        return buf;
    }

    char path[PATH_MAX_LEN] = {0};
    append_abs_path_prefix(path, sizeof(path)-1, file);
    file_to_unix_path(path);

    char srclist[2*strlen(path)];
    path_str_to_strlist(srclist, sizeof(srclist), path);

    char finallist[sizeof(srclist)];
    memset(finallist, 0, sizeof(finallist));

    char *pos;
    for(pos = srclist;
        *pos;
        pos = strlist_get_next(pos))
    {
        if( !strcmp(pos, "./") || !strcmp(pos, ".") )
        {
            // Nothing to do.
        }
        else if( !strcmp(pos, "../") || !strcmp(pos, "..") )
        {
            if( !( strlist_get_count(finallist) == 1 && have_root_path(finallist) ) )
                strlist_remove_last(finallist);
        }
        else
        {
            strlist_append(finallist, sizeof(finallist), pos, -1);
        }
    }

    strlist_to_path_str(buf, bufsz, finallist);
    return buf;
}
//------------------------------------------------------------------------------
static
bool is_pathdir_equal(const char *dir1, const char *dir2)
{
    if( !strcmp(dir1, dir2) ) return true;

    unsigned len1 = strlen(dir1);
    unsigned len2 = strlen(dir2);
    return ( len1 == len2 + 1 && dir1[len1-1] == '/' && !strncmp(dir1, dir2, len2) ) ||
           ( len2 == len1 + 1 && dir2[len2-1] == '/' && !strncmp(dir1, dir2, len1) );
}
//------------------------------------------------------------------------------
static
void find_pathlist_diff_pos(char *list1, char *list2, char **pos1, char **pos2)
{
    for(*pos1 = list1, *pos2 = list2;
        **pos1 && **pos2 && is_pathdir_equal(*pos1, *pos2);
        *pos1 = strlist_get_next(*pos1), *pos2 = strlist_get_next(*pos2))
    {}
}
//------------------------------------------------------------------------------
char* file_get_rel_path(char *buf, size_t bufsz, const char *file, const char *base)
{
    /**
     * Calculate relative path of a file path name.
     *
     * @param buf   A buffer to receive the result.
     * @param bufsz Size of the output buffer.
     * @param file  The file path name to inquiry.
     * @param base  The path to be the calculation base.
     *              This parameter can be NULL, and
     *              the current working directory will be used.
     * @return The output buffer.
     */
    if( !bufsz ) return buf;

    if( !file[0] )
    {
        buf[0] = 0;
        return buf;
    }

    // Get absolute path.

    char absbase[PATH_MAX_LEN] = {0};
    if( base && strlen(base) )
        file_get_abs_path(absbase, sizeof(absbase), base);
    else
        strncpy(absbase, file_get_working_dir(), sizeof(absbase)-1);

    char absfile[PATH_MAX_LEN] = {0};
    file_get_abs_path(absfile, sizeof(absfile), file);

    // Get path list.

    char baselist[2*strlen(absfile)];
    path_str_to_strlist(baselist, sizeof(baselist), absbase);

    char filelist[2*strlen(absfile)];
    path_str_to_strlist(filelist, sizeof(filelist), absfile);

    char finallist[ sizeof(filelist) + sizeof(baselist) ];
    memset(finallist, 0, sizeof(finallist));

    // Use all file path as the result if the first directory does not match.
    if( !is_pathdir_equal(baselist, filelist) )
    {
        memset(buf, 0, bufsz);
        strncpy(buf, absfile, bufsz-1);
        return buf;
    }

    // Skip matched directories and analysis the rests.

    char *basepos, *filepos;
    find_pathlist_diff_pos(baselist, filelist, &basepos, &filepos);

    if( !*basepos && !*filepos )
    {
        strlist_append(finallist, sizeof(finallist), "./", -1);
    }
    else
    {
        for(; *basepos; basepos = strlist_get_next(basepos))
            strlist_append(finallist, sizeof(finallist), "../", -1);

        for(; *filepos; filepos = strlist_get_next(filepos))
            strlist_append(finallist, sizeof(finallist), filepos, -1);
    }

    // Merge the final list to a string.
    strlist_to_path_str(buf, bufsz, finallist);

    // Check and modify the last slash.
    char *file_lastpos  = &absfile[strlen(absfile)-1];
    char *final_lastpos = &buf[strlen(buf)-1];
    if( *file_lastpos != '/' && *final_lastpos == '/' )
        *final_lastpos = 0;

    return buf;
}
//------------------------------------------------------------------------------
//---- File system inquiry -----------------------------------------------------
//------------------------------------------------------------------------------
bool file_is_file_existed(const char *file)
{
    /**
     * Check if a file existed.
     */
#if   defined(__linux__)
    struct stat state;
    if( stat(file, &state) ) return false;

    DIR *fdir = opendir(file);
    closedir(fdir);

    return !fdir;
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1] = {0};
    utf8_to_winstr(tstr, PATH_MAX_LEN, file);

    // Remove the last slash
    if( tstr[_tcslen(tstr)-1] == '/' || tstr[_tcslen(tstr)-1] == '\\' )
        tstr[_tcslen(tstr)-1] = 0;

    WIN32_FIND_DATA finddata;
    HANDLE hfind = FindFirstFile(tstr, &finddata);
    if( hfind == INVALID_HANDLE_VALUE ) return false;
    FindClose(hfind);

    return !( finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool file_is_dir_existed(const char *dir)
{
    /**
     * Check if a directory existed.
     */
#if   defined(__linux__)
    struct stat state;
    if( stat(dir, &state) ) return false;

    DIR *fdir = opendir(dir);
    closedir(fdir);

    return fdir;
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1] = {0};
    utf8_to_winstr(tstr, PATH_MAX_LEN, dir);

    // Remove the last slash
    if( tstr[_tcslen(tstr)-1] == '/' || tstr[_tcslen(tstr)-1] == '\\' )
        tstr[_tcslen(tstr)-1] = 0;

    WIN32_FIND_DATA finddata;
    HANDLE hfind = FindFirstFile(tstr, &finddata);
    if( hfind == INVALID_HANDLE_VALUE ) return false;
    FindClose(hfind);

    return finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool file_is_dir_empty(const char *dir)
{
    /**
     * Check is a directory empty.
     */
    DIR *fdir = opendir(dir);
    if( !fdir ) return false;

    bool found_subitem = false;

    struct dirent *item;
    for(item = readdir(fdir); item && !found_subitem; item = readdir(fdir))
    {
        found_subitem = strcmp(item->d_name, "." ) &&
                        strcmp(item->d_name, "..");
    }

    closedir(fdir);

    return !found_subitem;
}
//------------------------------------------------------------------------------
//---- File system operation ---------------------------------------------------
//------------------------------------------------------------------------------
bool file_create_dir(const char *dir)
{
    /**
     * Create a directory.
     *
     * @param dir The directory to be created.
     * @return TRUE if succeed; and FALSE if not.
     */
#if   defined(__linux__)
    mode_t dmode = S_IRWXU | S_IRWXG | S_IRWXO;
    mode_t umode = umask(0);  // This will get and also change umask value.
    umask(umode);  // Restore umask value.

    mode_t mode = dmode & ~umode;
    if( !mkdir(dir, mode) ) return true;

    return ( errno == EEXIST )?( file_is_dir_existed(dir) ):( false );
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1] = {0};
    utf8_to_winstr(tstr, PATH_MAX_LEN, dir);

    if( CreateDirectory(tstr, NULL) ) return true;

    return ( GetLastError() == ERROR_ALREADY_EXISTS )?
           ( file_is_dir_existed(dir) ):( false );
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool file_remove_dir(const char *dir)
{
    /**
     * Remove a directory.
     *
     * @param dir The directory to be removed.
     * @return TRUE if succeed; and FALSE if not.
     *
     * @remarks A directory cannot be removed if it is not empty.
     */
#if   defined(__linux__)
    return !rmdir(dir);
#elif defined(_WIN32)
    TCHAR tstr[PATH_MAX_LEN+1] = {0};
    utf8_to_winstr(tstr, PATH_MAX_LEN, dir);

    if( RemoveDirectory(tstr) ) return true;

    // If the directory cannot be removed, check if it existed.
    if( !file_is_dir_existed(dir) ) return false;

    // If the directory cannot be removed, try to change its permission and remove it again then.
    if( _tchmod(tstr, _S_IREAD | _S_IWRITE) ) return false;
    return RemoveDirectory(tstr);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool file_create_path(const char *path)
{
    /**
     * Create a directory, and also it parents if they do not existed.
     *
     * @param path The directory (and its path) to be created.
     * @return TRUE if succeed; and FALSE if not.
     */
    if( !strlen(path) ) return false;

    if( file_create_dir(path) ) return true;

    char parent[PATH_MAX_LEN] = {0};
    file_get_file_dir(parent, sizeof(parent)-1, path);
    if( !file_create_path(parent) ) return false;

    return file_create_dir(path);
}
//------------------------------------------------------------------------------
static
char* get_child_full_name(char *buf, size_t size, const char *parentname, const char *childname)
{
    memset(buf, 0, size);
    strncpy(buf, parentname, size-1);
    file_to_unix_path(buf);

    if( buf[strlen(buf)-1] != '/' )
        strncat(buf, "/", size-strlen(buf)-1);
    strncat(buf, childname, size-strlen(buf)-1);

    return buf;
}
//------------------------------------------------------------------------------
bool file_remove_recursive(const char *name)
{
    /**
     * Remove file or directories recursively.
     *
     * @param name The name of target to be removed.
     * @return TRUE if succeed; and FALSE if not.
     */
    if( !strlen(name) ) return false;

    if( file_is_file_existed(name) ) return !remove(name);
    if( !file_is_dir_existed(name) || file_remove_dir(name) ) return true;

    DIR *fdir = opendir(name);
    if( !fdir ) return false;

    struct dirent *item;
    for(item = readdir(fdir); item; item = readdir(fdir))
    {
        if( !strcmp(item->d_name, ".") || !strcmp(item->d_name, "..") )
            continue;

        char childname[PATH_MAX_LEN] = {0};
        get_child_full_name(childname, sizeof(childname), name, item->d_name);

        if( item->d_type == DT_DIR )
        {
            if( !file_remove_recursive(childname) )
                break;
        }
        else
        {
            if( remove(childname) )
                break;
        }
    }

    closedir(fdir);

    return file_remove_dir(name);
}
//------------------------------------------------------------------------------
//---- Files enumeration -------------------------------------------------------
//------------------------------------------------------------------------------
static
file_list_t* file_list_create(void)
{
    file_list_t *list = malloc(sizeof(file_list_t));
    if( list )
    {
        list->count = 0;
        list->names = NULL;
    }

    return list;
}
//------------------------------------------------------------------------------
void file_list_free(file_list_t *list)
{
    /**
     * @memberof file_list_t
     * @brief Release file name list.
     */
    if( list )
    {
        for(unsigned i=0; i<list->count; ++i)
            free(list->names[i]);
        free(list->names);
        free(list);
    }
}
//------------------------------------------------------------------------------
#ifdef _WIN32
static
char* strdup_win32(const char *str)
{
    size_t size = strlen(str) + 1;
    char *res = malloc(size);
    if( res ) memcpy(res, str, size);
    return res;
}
#endif
//------------------------------------------------------------------------------
static
bool file_list_push_back(file_list_t *list, const char *name)
{
    bool res = false;
    do
    {
        size_t newsize = sizeof(list->names[0])*( list->count + 1 );
        char **newarr = ( list->names )?( realloc(list->names, newsize) ):( malloc(newsize) );
        if( !newarr ) break;

        list->names = newarr;
#ifdef _WIN32
        if(( list->names[list->count] = strdup_win32(name) ))
            ++list->count;
#else
        if(( list->names[list->count] = strdup(name) ))
            ++list->count;
#endif

        res = true;
    } while(false);

    return res;
}
//------------------------------------------------------------------------------
static
bool file_list_push_list_back(file_list_t *list, const file_list_t *src)
{
    if( !src ) return true;

    for(unsigned i=0; i<src->count; ++i)
    {
        if( !file_list_push_back(list, src->names[i]) )
            return false;
    }

    return true;
}
//------------------------------------------------------------------------------
file_list_t* file_list_files(const char *dir, int flags)
{
    /**
     * List files under a specified directory.
     *
     * @param dir   The specified directory.
     * @param flags Combination of export operations,
     *              see ::TEnumFlags for more information.
     * @return A list of files and directories be enumerated.
     *         And the list must be released by ::file_list_free when not using any more.
     *
     * @remarks Function will throw exceptions if a directory cannot be enumerated.
     */
    file_list_t *res  = NULL;
    DIR *fdir = NULL;

    JMPBK_BEGIN
    {
        fdir = opendir(dir);
        if( !fdir ) JMPBK_THROW(0);

        res = file_list_create();
        if( !res ) JMPBK_THROW(0);

        struct dirent *item;
        for(item = readdir(fdir); item; item = readdir(fdir))
        {
            if( !strcmp(item->d_name, ".") || !strcmp(item->d_name, "..") )
                continue;

            bool isdir = ( item->d_type == DT_DIR );

            char itemname[PATH_MAX_LEN] = {0};
            strncpy(itemname, item->d_name, sizeof(itemname)-1);

            char fullname[PATH_MAX_LEN] = {0};
            get_child_full_name(fullname, sizeof(fullname), dir, itemname);

            if( flags & FILE_ENUM_RECURSIVE )
            {
                file_list_t *list = file_list_files(fullname, flags);
                bool pushres = file_list_push_list_back(res, list);
                file_list_free(list);
                if( !pushres ) JMPBK_THROW(0);
            }

            char exportname[PATH_MAX_LEN] = {0};
            strncpy(exportname,
                    ( flags & FILE_ENUM_PREFIX )?( fullname ):( itemname ),
                    sizeof(exportname)-1);

            if(( isdir )&& ( flags & FILE_ENUM_DIRSLASH ))
                strncat(exportname, "/", sizeof(exportname)-strlen(exportname)-1);

            bool not_export = ( ( flags & FILE_ENUM_DIRONLY  ) && !isdir ) ||
                              ( ( flags & FILE_ENUM_FILEONLY ) &&  isdir );
            if( !not_export && !file_list_push_back(res, exportname) )
                JMPBK_THROW(0);
        }
    }
    JMPBK_CATCH_ALL
    {
        file_list_free(res);
        res = NULL;
    }
    JMPBK_END

    if( fdir ) closedir(fdir);

    return res;
}
//------------------------------------------------------------------------------

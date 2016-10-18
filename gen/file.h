/**
 * @file
 * @brief     File utilities.
 * @details   Operations about file.
 * @author    王文佑
 * @date      2014.01.22
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
/**
 * @file
 * @note @li This module use UNIX style path separator "/" to all function results,
 *           although the Windows style path can be identified.
 *       @li Word of "path" in this module means a file path string that ended
 *           with character "/".
 *       @li Word of "directory" in this module means a file path string that ended
 *           without character "/".
 *       @li The file name extension means characters after the last dot "." (including the dot)
 *           of a file name.
 *       @li The file name total extension means characters after the first dot "." (including the dot)
 *           of a file name (excluding the path).
 */
#ifndef _GEN_FILE_H_
#define _GEN_FILE_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <stdexcept>
#include <string>
#include <vector>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Path style
 * @{
 */
void file_to_unix_path   (char *path);
void file_to_windows_path(char *path);
/**
 * @}
 */

/**
 * @name Properties inquiry
 * @{
 */
size_t file_get_max_path_len(void);
/**
 * @}
 */

/**
 * @name Path inquiry
 * @{
 */
const char* file_get_working_dir(void);
const char* file_get_home_dir(void);
const char* file_get_temp_dir(void);
/**
 * @}
 */

/**
 * @name Path name operation
 * @{
 */
char* file_set_file_ext    (char *buf, size_t bufsz, const char *file, const char *ext);
char* file_get_file_ext    (char *buf, size_t bufsz, const char *file);
char* file_set_file_ext_all(char *buf, size_t bufsz, const char *file, const char *ext);
char* file_get_file_ext_all(char *buf, size_t bufsz, const char *file);
char* file_get_file_name   (char *buf, size_t bufsz, const char *file);
char* file_get_file_dir    (char *buf, size_t bufsz, const char *file);
char* file_get_file_path   (char *buf, size_t bufsz, const char *file);
char* file_get_abs_path    (char *buf, size_t bufsz, const char *file);
char* file_get_rel_path    (char *buf, size_t bufsz, const char *file, const char *base);
/**
 * @}
 */

/**
 * @name File system inquiry
 * @{
 */
bool file_is_file_existed(const char *file);
bool file_is_dir_existed (const char *dir);
bool file_is_dir_empty   (const char *dir);
/**
 * @}
 */

/**
 * @name File system operation
 * @{
 */
bool file_create_dir(const char *dir);
bool file_remove_dir(const char *dir);
bool file_create_path(const char *path);
bool file_remove_recursive(const char *name);
/**
 * @}
 */

/**
 * @class file_list_t
 * @brief File name list.
 */
typedef struct file_list_t
{
    unsigned   count;  ///< Number of strings inside.
    char     **names;  ///< Array of C-style strings.
} file_list_t;

void file_list_free(file_list_t *list);

/**
 * @brief Files enumeration flags.
 */
enum file_enum_flags_t
{
    FILE_ENUM_NAMEONLY  = 0,     ///< Export name only (default).
    FILE_ENUM_RECURSIVE = 1<<0,  ///< Export name recursively.
    FILE_ENUM_FILEONLY  = 1<<1,  ///< Export files only.
    FILE_ENUM_DIRONLY   = 1<<2,  ///< Export directories only.
    FILE_ENUM_PREFIX    = 1<<3,  ///< Export name with path prefix.
    FILE_ENUM_DIRSLASH  = 1<<4,  ///< Append slash on the end of name of directories.
};

/**
 * @name Files enumeration
 * @{
 */
file_list_t* file_list_files(const char *dir, int flags);
/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus
/**
 * C++ wrapper
 */
namespace file
{

/**
 * @name Path style
 * @{
 */

inline
std::string ToUnixPath(const std::string &path)
{
    /// @see ::file_to_unix_path
    std::string res = path;
    file_to_unix_path((char*)res.c_str());
    return res;
}

inline
std::string ToWindowsPath(const std::string &path)
{
    /// @see ::file_to_windows_path
    std::string res = path;
    file_to_windows_path((char*)res.c_str());
    return res;
}

/**
 * @}
 */

/**
 * @name Properties inquiry
 * @{
 */
inline size_t GetMaxPathLen() { return file_get_max_path_len(); }  ///< @see ::file_get_max_path_len
/**
 * @}
 */

/**
 * @name Path inquiry
 * @{
 */
inline std::string GetWorkingDir() { return file_get_working_dir(); }  ///< @see ::file_get_working_dir
inline std::string GetHomeDir()    { return file_get_home_dir(); }     ///< @see ::file_get_home_dir
inline std::string GetTempDir()    { return file_get_temp_dir(); }     ///< @see ::file_get_temp_dir
/**
 * @}
 */

/**
 * @name Path name operation
 * @{
 */

inline
std::string SetFileExt(const std::string &file, const std::string ext)
{
    /// @see ::file_set_file_ext
    char buf[GetMaxPathLen()];
    return file_set_file_ext(buf, sizeof(buf), file.c_str(), ext.c_str());
}

inline
std::string GetFileExt(const std::string &file)
{
    /// @see ::file_get_file_ext
    char buf[GetMaxPathLen()];
    return file_get_file_ext(buf, sizeof(buf), file.c_str());
}

inline
std::string SetFileExtAll(const std::string &file, const std::string ext)
{
    /// @see ::file_set_file_ext_all
    char buf[GetMaxPathLen()];
    return file_set_file_ext_all(buf, sizeof(buf), file.c_str(), ext.c_str());
}

inline
std::string GetFileExtAll(const std::string &file)
{
    /// @see ::file_get_file_ext_all
    char buf[GetMaxPathLen()];
    return file_get_file_ext_all(buf, sizeof(buf), file.c_str());
}

inline
std::string GetFileName(const std::string &file)
{
    /// @see ::file_get_file_name
    char buf[GetMaxPathLen()];
    return file_get_file_name(buf, sizeof(buf), file.c_str());
}

inline
std::string GetFileDir(const std::string &file)
{
    /// @see ::file_get_file_dir
    char buf[GetMaxPathLen()];
    return file_get_file_dir(buf, sizeof(buf), file.c_str());
}

inline
std::string GetFilePath(const std::string &file)
{
    /// @see ::file_get_file_path
    char buf[GetMaxPathLen()];
    return file_get_file_path(buf, sizeof(buf), file.c_str());
}

inline
std::string GetAbsPath(const std::string &file)
{
    /// @see ::file_get_abs_path
    char buf[GetMaxPathLen()];
    return file_get_abs_path(buf, sizeof(buf), file.c_str());
}

inline
std::string GetRelPath(const std::string &file, const std::string &base)
{
    /// @see ::file_get_rel_path
    char buf[GetMaxPathLen()];
    return file_get_rel_path(buf, sizeof(buf), file.c_str(), base.c_str());
}

/**
 * @}
 */

/**
 * @name File system inquiry
 * @{
 */
inline bool IsFileExisted(const std::string &file) { return file_is_file_existed(file.c_str()); }  ///< @see ::file_is_file_existed
inline bool IsDirExisted (const std::string &dir)  { return file_is_dir_existed (dir .c_str()); }  ///< @see ::file_is_dir_existed
inline bool IsDirEmpty   (const std::string &dir)  { return file_is_dir_empty   (dir .c_str()); }  ///< @see ::file_is_dir_empty
/**
 * @}
 */

/**
 * @name File system operation
 * @{
 */
inline bool CreateDir      (const std::string &dir)  { return file_create_dir(dir.c_str()); }           ///< @see ::file_create_dir
inline bool RemoveDir      (const std::string &dir)  { return file_remove_dir(dir.c_str()); }           ///< @see ::file_remove_dir
inline bool CreatePath     (const std::string &path) { return file_create_path(path.c_str()); }         ///< @see ::file_create_path
inline bool RemoveRecursive(const std::string &name) { return file_remove_recursive(name.c_str()); }    ///< @see ::file_remove_recursive
/**
 * @}
 */

/**
 * @brief Files enumeration flags.
 */
enum TEnumFlags
{
    NAMEONLY  = FILE_ENUM_NAMEONLY,   ///< Export name only (default).
    RECURSIVE = FILE_ENUM_RECURSIVE,  ///< Export name recursively.
    FILEONLY  = FILE_ENUM_FILEONLY,   ///< Export files only.
    DIRONLY   = FILE_ENUM_DIRONLY,    ///< Export directories only.
    PREFIX    = FILE_ENUM_PREFIX,     ///< Export name with path prefix.
    DIRSLASH  = FILE_ENUM_DIRSLASH,   ///< Append slash on the end of name of directories.
};

/**
 * @name Files enumeration
 * @{
 */
inline
std::vector<std::string> ListFiles(const std::string &dir, int flags)
{
    /**
     * Enumerate files under a specified directory.
     *
     * @param dir   The specified directory.
     * @param flags Combination of export operations,
     *              see ::TEnumFlags for more information.
     * @return An array of files and directories be enumerated.
     *
     * @remarks Function will throw exceptions if a directory cannot be enumerated.
     */
    file_list_t *list = file_list_files(dir.c_str(), flags);
    if( !list ) throw std::runtime_error(__func__);

    std::vector<std::string> res;
    try
    {
        res.reserve(list->count);
        for(unsigned i=0; i<list->count; ++i)
            res.push_back(list->names[i]);

        file_list_free(list);
    }
    catch(...)
    {
        file_list_free(list);
        throw;
    }

    return res;
}
/**
 * @}
 */

}  // namespace file
#endif

#endif

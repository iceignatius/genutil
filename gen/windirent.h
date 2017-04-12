/**
 * @file
 * @brief     POSIX dirent functions on Windows.
 * @author    王文佑
 * @date      2016.06.30
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_WINDIRENT_H_
#define _GEN_WINDIRENT_H_

#ifndef _WIN32
#error This module can be used on Windows only!
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DIR enumeration object.
 */
typedef struct DIR DIR;

/**
 * File type
 */
enum
{
    DT_BLK,      ///< This is a block device.
    DT_CHR,      ///< This is a character device.
    DT_DIR,      ///< This is a directory.
    DT_FIFO,     ///< This is a named pipe (FIFO).
    DT_LNK,      ///< This is a symbolic link.
    DT_REG,      ///< This is a regular file.
    DT_SOCK,     ///< This is a UNIX domain socket.
    DT_UNKNOWN,  ///< The file type could not be determined.
};

/**
 * dirent.
 */
struct dirent
{
    unsigned       d_ino;         ///< Not used!
    unsigned       d_off;         ///< Not used!
    unsigned short d_reclen;      ///< Length of this record.
    unsigned char  d_type;        ///< File type; not supported by all file system types.
    char           d_name[1024];  ///< Null-terminated file name.
};

DIR* opendir(const char *name);
int closedir(DIR *dir);

struct dirent *readdir(DIR *dir);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

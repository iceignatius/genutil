#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux__
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef _WIN32
    #include "utf.h"
#endif

#include "shrdmem.h"

//------------------------------------------------------------------------------
void shrdmem_init(shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief Constructor.
     *
     * @param self Object instance.
     */
    assert( self );

    memset(self, 0, sizeof(shrdmem_t));
#ifdef __linux__
    self->fd = -1;
#endif
}
//------------------------------------------------------------------------------
void shrdmem_deinit(shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief Destructor.
     *
     * @param self Object instance.
     */
    assert( self );

    shrdmem_close(self);
    memset(self, 0, sizeof(shrdmem_t));
}
//------------------------------------------------------------------------------
shrdmem_t* shrdmem_create(void)
{
    /**
     * @memberof shrdmem_t
     * @brief 創建動態的物件。
     *
     * @return The object reference of succeed; and NULL if failed.
     */
    shrdmem_t *self = malloc(sizeof(shrdmem_t));
    if( self ) shrdmem_init(self);
    return self;
}
//------------------------------------------------------------------------------
shrdmem_t* shrdmem_create_open(const char* RESTRICT name, size_t size, bool fail_if_existed)
{
    /**
     * @memberof shrdmem_t
     * @brief 創建動態的物件，並建立全域的共用記憶體空間。
     *
     * @param name            指定的共用記憶體名稱。
     * @param size            欲建立的共用空間大小，注意此值僅為建議值。
     * @param fail_if_existed 指定當的具名共用記憶體已存在時的反應：
     *                        @arg TRUE，則會返回失敗結果；
     *                        @arg FALSE，則會開啟該共用空間，並忽略 @a size 參數。
     * @return The object reference of succeed; and NULL if failed.
     *
     * @warning 注意實際建立出來的共用空間大小可能會不同於指定的大小，
     *          一般狀況下實際大小可能會略大於指定值，但也有可能在某些狀況下小於指定值。
     *          使用者應在共用記憶體空間創建完成後自行檢查空間大小。
     */
    shrdmem_t *self = malloc(sizeof(shrdmem_t));
    if( self )
    {
        shrdmem_init(self);
        if( !shrdmem_open(self, name, size, fail_if_existed) )
        {
            free(self);
            self = NULL;
        }
    }

    return self;
}
//------------------------------------------------------------------------------
shrdmem_t* shrdmem_create_open_existed(const char* RESTRICT name)
{
    /**
     * @memberof shrdmem_t
     * @brief 創建動態的物件，並開啟已存在的具名共用記憶體。
     *
     * @param name 欲開啟的已存共用記憶體名稱。
     * @return The object reference of succeed; and NULL if failed.
     */
    shrdmem_t *self = malloc(sizeof(shrdmem_t));
    if( self )
    {
        shrdmem_init(self);
        if( !shrdmem_open_existed(self, name) )
        {
            free(self);
            self = NULL;
        }
    }

    return self;
}
//------------------------------------------------------------------------------
void shrdmem_release(shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief 釋放動態的物件。
     *
     * @param self Object instance.
     */
    if( self )
    {
        shrdmem_deinit(self);
        free(self);
    }
}
//------------------------------------------------------------------------------
void shrdmem_release_s(shrdmem_t** RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief 釋放動態的物件，並重設物件指標為NULL。
     *
     * @param self Reference of the object instance.
     */
    if( self )
    {
        shrdmem_release(*self);
        *self = NULL;
    }
}
//------------------------------------------------------------------------------
void shrdmem_set_zeros(shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief 將緩衝區的資料全設定為零。
     *
     * @param self Object instance.
     */
    if( self && self->buf )
        memset(self->buf, 0, self->size);
}
//------------------------------------------------------------------------------
bool shrdmem_open(shrdmem_t* RESTRICT self, const char* RESTRICT name, size_t size, bool fail_if_existed)
{
    /**
     * @memberof shrdmem_t
     * @brief 建立一個具名的共用記憶體。
     * @param self            物件指標。
     * @param name            指定的共用記憶體名稱。
     * @param size            欲建立的共用空間大小，注意此值僅為建議值。
     * @param fail_if_existed 指定當的具名共用記憶體已存在時的反應：
     *                        @arg TRUE，則會返回失敗結果；
     *                        @arg FALSE，則會開啟該共用空間，並忽略 @a size 參數。
     * @return 傳回共用記憶體是否開啟成功的邏輯。
     *
     * @warning 注意實際建立出來的共用空間大小可能會不同於指定的大小，
     *          一般狀況下實際大小可能會略大於指定值，但也有可能在某些狀況下小於指定值。
     *          使用者應在共用記憶體空間創建完成後自行檢查空間大小。
     */
    bool bsucceed = false;

#if   defined(__linux__)
    char        pathname[512];
    int         flags = O_RDWR | O_CREAT | ( fail_if_existed ? O_EXCL : 0 );
    struct stat filestat;

    if( !self || !name ) return false;

    shrdmem_close(self);

    do
    {
        // Create file
        snprintf(pathname, sizeof(pathname), "%s%s", SHRDMEM_SHRDPATH, name);
        self->fd = open(pathname, flags, S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP);
        if( self->fd < 0 ) break;
        self->remove_file_when_close = true;

        // Change file size
        if( ftruncate(self->fd, size) ) break;
        if( fstat(self->fd, &filestat) ) break;

        // File mapping
        self->buf = mmap(NULL,
                         filestat.st_size,
                         PROT_READ | PROT_WRITE,
                         MAP_FILE | MAP_SHARED,
                         self->fd,
                         0);
        if( self->buf == MAP_FAILED )
        {
            self->buf = NULL;
            break;
        }
        self->size = filestat.st_size;

        bsucceed = true;
    } while( false );
#elif defined(_WIN32)
    DWORD                    error_code;
    MEMORY_BASIC_INFORMATION info;
    size_t                   infosz;

    if( !self || !name ) return false;

    shrdmem_close(self);

    do
    {
        // Translate string to Windows format
        TCHAR winname[260];
        if( !utf8_to_winchar(winname, sizeof(winname), name) )
            break;
        // Create file mapping
        self->hmap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, winname);
        error_code = GetLastError();
        if( !self->hmap ) break;
        if(( mode == shrdmem_mode_fail_if_existed )&&( error_code == ERROR_ALREADY_EXISTS )) break;

        // Set map view
        self->buf = MapViewOfFile(self->hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if( !self->buf ) break;

        // Get memory size
        infosz = VirtualQuery(self->buf, &info, sizeof(info));
        assert( infosz >= sizeof(info) );
        self->size = info.RegionSize;

        bsucceed = true;
    } while( false );
#else
    #error No implementation on this platform!
#endif

    if( !bsucceed ) shrdmem_close(self);

    return bsucceed;
}
//------------------------------------------------------------------------------
bool shrdmem_open_existed(shrdmem_t* RESTRICT self, const char* RESTRICT name)
{
    /**
     * @memberof shrdmem_t
     * @brief 開啟已存在的具名共用記憶體。
     *
     * @param self Object instance.
     * @param name 指定的共用記憶體名稱。
     * @return TRUE if succeed; and FALSE if failed.
     */
    bool bsucceed = false;

#if   defined(__linux__)
    char        pathname[512];
    int         flags = O_RDWR;
    struct stat filestat;

    if( !self || !name ) return false;

    shrdmem_close(self);

    do
    {
        // open file
        snprintf(pathname, sizeof(pathname), "%s%s", SHRDMEM_SHRDPATH, name);
        self->fd = open(pathname, flags);
        if( self->fd < 0 ) break;

        // Get file status
        if( fstat(self->fd, &filestat) ) break;

        // File mapping
        self->buf = mmap(NULL,
                         filestat.st_size,
                         PROT_READ | PROT_WRITE,
                         MAP_FILE | MAP_SHARED,
                         self->fd,
                         0);
        if( self->buf == MAP_FAILED )
        {
            self->buf = NULL;
            break;
        }
        self->size = filestat.st_size;

        bsucceed = true;
    } while( false );
#elif defined(_WIN32)
    MEMORY_BASIC_INFORMATION info;
    size_t                   infosz;

    if( !self || !name ) return false;

    shrdmem_close(self);

    do
    {
        TCHAR winname[260];
        if( !utf8_to_winchar(winname, sizeof(winname), name) )
            break;

        self->hmap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, winname);
        if( !self->hmap ) break;

        self->buf = MapViewOfFile(self->hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if( !self->buf ) break;

        infosz = VirtualQuery(self->buf, &info, sizeof(info));
        assert( infosz >= sizeof(info) );
        self->size = info.RegionSize;

        bsucceed = true;
    } while( false );
#else
    #error No implementation on this platform!
#endif

    if( !bsucceed ) shrdmem_close(self);

    return bsucceed;
}
//------------------------------------------------------------------------------
#ifdef __linux__
static
char* get_filename_by_fd(char *buf, size_t bufsz, int fd)
{
    // Get file descriptor information.
    char linkname[32];
    snprintf(linkname, sizeof(linkname), "/proc/self/fd/%d", fd);

    // Get filename.
    if( 0 >= readlink(linkname, buf, bufsz) ) return NULL;

    // Verify file information.
    struct stat stat1, stat2;
    if( stat (buf, &stat1) ) return NULL;
    if( fstat(fd , &stat2) ) return NULL;
    if( stat1.st_dev != stat2.st_dev ||
        stat1.st_ino != stat2.st_ino )
    {
        return NULL;
    }

    return buf;
}
//------------------------------------------------------------------------------
static
void unlink_by_fd(int fd)
{
    char filename[512] = {0};
    if( get_filename_by_fd(filename, sizeof(filename), fd) )
        unlink(filename);
}
#endif
//------------------------------------------------------------------------------
void shrdmem_close(shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief 關閉物件中開啟的共用記憶體。
     *
     * @param self Object instance.
     */
    bool unmap_result;
    bool close_result;

    if( self )
    {
#if   defined(__linux__)
        if( self->buf )
        {
            unmap_result = munmap(self->buf, self->size);
            self->buf    = NULL;
            self->size   = 0;
            assert( !unmap_result );
        }
        if( self->fd >= 0 )
        {
            if( self->remove_file_when_close )
            {
                self->remove_file_when_close = false;
                unlink_by_fd(self->fd);
            }
            close_result = close(self->fd);
            self->fd     = -1;
            assert( !close_result );
        }
#elif defined(_WIN32)
        if( self->buf )
        {
            unmap_result = UnmapViewOfFile(self->buf);
            self->buf    = NULL;
            self->size   = 0;
            assert( unmap_result );
        }
        if( self->hmap )
        {
            close_result = CloseHandle(self->hmap);
            self->hmap   = NULL;
            assert( close_result );
        }
#else
    #error No implementation on this platform!
#endif
    }
}
//------------------------------------------------------------------------------
bool shrdmem_is_opened(const shrdmem_t* RESTRICT self)
{
    /**
     * @memberof shrdmem_t
     * @brief 檢查物件是否已在開啟狀態。
     *
     * @param self Object instance.
     * @return TRUE if it is opened; and FALSE if not.
     */
    return self && self->buf;
}
//------------------------------------------------------------------------------
bool shrdmem_query_existed(const char* RESTRICT name)
{
    /**
     * @memberof shrdmem_t
     * @static
     * @brief 檢查指定的名稱是否已存在。
     *
     * @param name 指定的共用記憶體名稱。
     * @return TRUE if the shared memory with the name is existed; and FALSE if not.
     */
#if   defined(__linux__)
    char pathname[512];
    int  fd;

    snprintf(pathname, sizeof(pathname), "%s%s", SHRDMEM_SHRDPATH, name);
    fd = open(pathname, O_RDONLY);
    close(fd);

    return fd >= 0;
#elif defined(_WIN32)
    HANDLE mapping;
    TCHAR  winname[260];

    if( !name ) return false;

    // Translate string to Windows format
    if( !utf8_to_winchar(winname, sizeof(winname), name) )
        return false;

    mapping = OpenFileMapping(FILE_MAP_READ, FALSE, winname);
    CloseHandle(mapping);

    return !!mapping;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------

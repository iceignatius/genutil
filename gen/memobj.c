#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef _WIN32
#include "utf.h"
#endif

#include "minmax.h"
#include "memobj.h"

//------------------------------------------------------------------------------
//---- Tools -------------------------------------------------------------------
//------------------------------------------------------------------------------
void* memfind(const void* src, size_t srcsz, const void* pattern, size_t patsz)
{
    /**
     * 在一段記憶體資料中從頭開始搜尋與指定資料吻合的位置。
     * @param src     被搜尋的緩衝區。
     * @param srcsz   被搜尋緩衝區大小。
     * @param pattern 搜尋目標。
     * @param patsz   搜尋目標大小。
     * @return 返回第一個搜尋到資料吻合的資料指標；若搜尋失敗則反回 NULL。
     */
    byte_t *dat;
    size_t  sizepass;

    while( true )
    {
        dat = memchr(src, *(byte_t*)pattern, srcsz);
        if( !dat ) break;
        sizepass = (size_t)dat - (size_t)src + 1;
        if( sizepass > srcsz ) break;

        if( 0 == memcmp(dat, pattern, patsz) ) return dat;

        src    = (byte_t*)src + sizepass;
        srcsz -= sizepass;
    }

    return NULL;
}
//------------------------------------------------------------------------------
void* memrfind(const void* src, size_t srcsz, const void* pattern, size_t patsz)
{
    /**
     * 在一段記憶體資料中從結尾開始搜尋與指定資料吻合的位置。
     * @param src     被搜尋的緩衝區。
     * @param srcsz   被搜尋緩衝區大小。
     * @param pattern 搜尋目標。
     * @param patsz   搜尋目標大小。
     * @return 返回第一個搜尋到資料吻合的資料指標；若搜尋失敗則反回 NULL。
     */
    byte_t *dat = (byte_t*)src + srcsz - patsz;

    while( true )
    {
        if(( *dat == *(byte_t*)pattern )&&( 0 == memcmp(dat, pattern, patsz) )) return dat;
        if( dat == src ) break;
        --dat;
    }

    return NULL;
}
//------------------------------------------------------------------------------
unsigned memfindcount(const void* src, size_t srcsz, const void* pattern, size_t patsz)
{
    /**
     * 搜尋一段記憶體資料中吻合某目標資料的片斷數量。
     * @param src     被搜尋的緩衝區。
     * @param srcsz   被搜尋緩衝區大小。
     * @param pattern 搜尋目標。
     * @param patsz   搜尋目標大小。
     * @return 返回緩衝區中吻合指定資料的片斷數量。
     */
    unsigned count = 0;

    const byte_t *pos0 = src;
    const byte_t *pos;
    while(( pos = memfind(pos0, srcsz, pattern, patsz) ))
    {
        ++count;

        size_t size = pos - pos0 + patsz;
        pos0  += size;
        srcsz -= size;
    }

    return count;
}
//------------------------------------------------------------------------------
size_t memfindreplace(void*       dest,
                      size_t      destsz,
                      const void* src,
                      size_t      srcsz,
                      const void* pattern,
                      size_t      patsz,
                      const void* target,
                      size_t      tarsz)
{
    /**
     * 搜尋一段記憶體中的特定資料，並將之取代為另一個指定的資料。
     * @param dest    接收結果資料的緩衝區。
     * @param destsz  接收結果資料的緩衝區大小。
     * @param src     被搜尋的緩衝區。
     * @param srcsz   被搜尋緩衝區大小。
     * @param pattern 搜尋目標。
     * @param patsz   搜尋目標大小。
     * @param target  取代搜尋目標的資料。
     * @param tarsz   取代搜尋目標的資料大小。
     * @return 成功時返回已寫入 @a dest 的資料數量；
     *         或當 @a dest 為 NULL 時返回輸出緩衝區所需的最小大小；
     *         或失敗時返回零。
     */
    byte_t       *_dest = dest;
    const byte_t *_src  = src;

    unsigned findcnt  = memfindcount(_src, srcsz, pattern, patsz);
    size_t   sizeneed = (long long)srcsz + findcnt*( (long long)tarsz - (long long)patsz );
    if( !_dest ) return sizeneed;
    if( destsz < sizeneed ) return 0;

    const byte_t *pos;
    do
    {
        pos = memfind(_src, srcsz, pattern, patsz);
        if( pos )
        {
            size_t blksz = pos - _src;
            memcpy(_dest, _src, blksz);
            _dest  += blksz;
            destsz -= blksz;

            memcpy(_dest, target, tarsz);
            _dest  += tarsz;
            destsz -= tarsz;

            blksz += patsz;
            _src  += blksz;
            srcsz -= blksz;
        }
        else
        {
            memcpy(_dest, _src, srcsz);
        }

    } while( pos );

    return sizeneed;
}
//------------------------------------------------------------------------------
size_t memobj_calc_recommended_size(size_t size)
{
    /*
     * 計算大於指定大小的最適合之緩衝區配置尺寸。
     */
    static const size_t size_max = ((size_t)-1)>>1;

    if     ( size > MEMOJB_BLOCKSZ_LARGE/2 )
    {
        if( size < size_max )
            size = ( size + MEMOJB_BLOCKSZ_LARGE - 1 ) & ~( MEMOJB_BLOCKSZ_LARGE - 1 );
    }
    else if( size > MEMOJB_BLOCKSZ_SMALL )
    {
        size = ( size + MEMOJB_BLOCKSZ_SMALL - 1 ) & ~( MEMOJB_BLOCKSZ_SMALL - 1 );
    }
    else
    {
        size = MEMOJB_BLOCKSZ_SMALL;
    }

    return size;
}
//------------------------------------------------------------------------------
static
FILE* file_open_readonly(const char* filename)
{
    assert( filename );

#if defined(_WIN32) && defined(UNICODE)
    wchar_t wcsname[260];
    if( !utf8_to_wcs(wcsname, sizeof(wcsname), filename) )
        return NULL;
    return _wfopen(wcsname, L"rb");
#else
    return fopen(filename, "rb");
#endif
}
//------------------------------------------------------------------------------
static
FILE* file_open_writable(const char* filename)
{
    assert( filename );

#if defined(_WIN32) && defined(UNICODE)
    wchar_t wcsname[260];
    if( !utf8_to_wcs(wcsname, sizeof(wcsname), filename) )
        return NULL;
    return _wfopen(wcsname, L"wb");
#else
    return fopen(filename, "wb");
#endif
}
//------------------------------------------------------------------------------
static
FILE* file_open_writable_text(const char* filename)
{
    assert( filename );

#if defined(_WIN32) && defined(UNICODE)
    wchar_t wcsname[260];
    if( !utf8_to_wcs(wcsname, sizeof(wcsname), filename) )
        return NULL;
    return _wfopen(wcsname, L"w");
#else
    return fopen(filename, "w");
#endif
}
//------------------------------------------------------------------------------
static
long file_get_size(FILE* file)
{
    /*
     * Get file size.
     * @param file Input a file handler.
     * @return A value great equal to zero if succeed;
     *         and a negative value if failed.
     * @note This function will change position of the file read-write head.
     */
    assert( file );

    if( fseek(file,0,SEEK_END) ) return -1;
    return ftell(file);
}
//------------------------------------------------------------------------------
static
size_t file_read_binary(FILE* file, byte_t* buffer, size_t size)
{
    assert( file );

    if( fseek(file,0,SEEK_SET) ) return 0;
    return fread(buffer, 1, size, file);
}
//------------------------------------------------------------------------------
static
bool file_write_binary(FILE* file, const byte_t* data, size_t size)
{
    assert( file && data );

    return size == fwrite(data, 1, size, file);
}
//------------------------------------------------------------------------------
static
void file_write_text(FILE* file, const byte_t* data, size_t size)
{
    /*
     * Translate data to text format and write to file.
     */
    static const char* const textgap = "    ";
    size_t i;

    assert( file && data );

    // Output data size
    fprintf(file, "Total Size : %lu (bytes)\n\n", (unsigned long)size);
    // Output address position label
    if( size )
    {
        fprintf(file,
                "%8s%s%s%s%s\n\n",
                " ",
                textgap,
                "00 01 02 03 04 05 06 07",
                textgap,
                "08 09 0A 0B 0C 0D 0E 0F");
    }
    // Output all binary data to text format
    for(i=0; i<size; ++i)
    {
        if( ( ( i ) % 16 ) == 0 ) fprintf(file, "%8.8lX%s", (unsigned long)i, textgap);

        fprintf(file, "%2.2X", (unsigned)data[i]);

        if     ( ( (i+1) % 16 ) == 0 ) fprintf(file, "\n");
        else if( ( (i+1) %  8 ) == 0 ) fprintf(file, "%s", textgap);
        else                           fprintf(file, " ");
    }
    if( ( i % 16 ) != 0 ) fprintf(file, "\n");
}
//------------------------------------------------------------------------------
//---- Memory Buffer Object ----------------------------------------------------
//------------------------------------------------------------------------------
static
void abort_if_alloc_fail(const void *buf)
{
    if( !buf )
    {
        fputs("ERROR: Memory allocation failed!", stderr);
        abort();
    }
}
//------------------------------------------------------------------------------
void mem_init(mem_t* RESTRICT self, size_t size)
{
    /**
     * @memberof mem_t
     * @brief Constructor.
     * @details Initialize object with an initial buffer size.
     *
     * @param self Object instance.
     * @param size The initial buffer size.
     */
    assert( self );

    self->size_total = memobj_calc_recommended_size(size);
    self->size       = size;
    assert( self->size_total >= self->size );

    self->buf = malloc(self->size_total);
    abort_if_alloc_fail(self->buf);
}
//------------------------------------------------------------------------------
void mem_init_import(mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof mem_t
     * @brief Constructor.
     * @details Initialize object and import data from another buffer.
     *
     * @param self   Object instance.
     * @param buffer Data to import from.
     * @param size   Size of data to import.
     */
    assert( self );

    size = buffer ? size : 0;
    mem_init(self, size);
    memcpy(self->buf, buffer, size);
}
//------------------------------------------------------------------------------
void mem_init_clone(mem_t* RESTRICT self, const mem_t* RESTRICT src)
{
    /**
     * @memberof mem_t
     * @brief Constructor.
     * @details Initialize object and import data from another @ref mem_t object.
     *
     * @param self Object instance.
     * @param src  Data to import from.
     */
    assert( self && src );

    mem_init(self, src->size);
    memcpy(self->buf, src->buf, src->size);
}
//------------------------------------------------------------------------------
void mem_init_move(mem_t* RESTRICT self, mem_t* RESTRICT src)
{
    /**
     * @memberof mem_t
     * @brief Constructor.
     * @details Initialize object and move data from another @ref mem_t object.
     *
     * @param self Object instance.
     * @param src  Data to move from.
     */
    assert( self && src );

    *self = *src;
    mem_init(src, 0);
}
//------------------------------------------------------------------------------
void mem_deinit(mem_t* RESTRICT self)
{
    /**
     * @memberof mem_t
     * @brief Destructor.
     */
    assert( self );
    free(self->buf);
}
//------------------------------------------------------------------------------
mem_t* mem_create(size_t size)
{
    /**
     * @memberof mem_t
     * @static
     * @brief 創建動態的物件。
     *
     * @param size The initial buffer size.
     * @return The object that created if succeed; and NULL if failed.
     */
    mem_t *obj = malloc(sizeof(mem_t));
    if( !obj ) return NULL;

    obj->size_total = memobj_calc_recommended_size(size);
    obj->size       = size;
    assert( obj->size_total >= obj->size );

    obj->buf = malloc(obj->size_total);
    if( !obj->buf )
    {
        free(obj);
        return NULL;
    }

    return obj;
}
//------------------------------------------------------------------------------
mem_t* mem_create_import(const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof mem_t
     * @static
     * @brief 創建動態的物件，並從其他緩衝區匯入資料。
     *
     * @param buffer Data to import from.
     * @param size   Size of data to import.
     * @return The object that created if succeed; and NULL if failed.
     */
    size = buffer ? size : 0;

    mem_t *obj = mem_create(size);
    if( !obj ) return NULL;

    memcpy(obj->buf, buffer, size);

    return obj;
}
//------------------------------------------------------------------------------
mem_t* mem_create_clone(const mem_t* RESTRICT src)
{
    /**
     * @memberof mem_t
     * @static
     * @brief 創建動態的物件，並從其他 mem 物件複製資料。
     *
     * @param src Data to import from.
     * @return The object that created if succeed; and NULL if failed.
     */
    if( !src ) return NULL;

    mem_t *obj = mem_create(src->size);
    if( !obj ) return NULL;

    memcpy(obj->buf, src->buf, src->size);

    return obj;
}
//------------------------------------------------------------------------------
mem_t* mem_create_move(mem_t* RESTRICT src)
{
    /**
     * @memberof mem_t
     * @static
     * @brief 創建動態的物件，並從其他 mem 物件搬移資料。
     *
     * @param src Data to move from.
     * @return The object that created if succeed; and NULL if failed.
     */
    if( !src ) return NULL;

    mem_t *obj = malloc(sizeof(mem_t));
    if( !obj ) return NULL;

    size_t bufsize = memobj_calc_recommended_size(0);
    byte_t *buf = malloc(bufsize);
    if( !buf )
    {
        free(obj);
        return NULL;
    }

    *obj = *src;

    src->size_total = bufsize;
    src->size       = 0;
    src->buf        = buf;

    return obj;
}
//------------------------------------------------------------------------------
mem_t* mem_create_load_file(const char* RESTRICT filename)
{
    /**
     * @memberof mem_t
     * @static
     * @brief 創建動態的物件，並從檔案讀入資料。
     *
     * @param filename The name of file to load data from.
     * @return The object that created if succeed; and NULL if failed.
     */
    mem_t *obj = mem_create(0);
    if( obj )
    {
        if( !mem_load_file(obj, filename) )
            mem_release_s(&obj);
    }
    return obj;
}
//------------------------------------------------------------------------------
void mem_release(mem_t* RESTRICT self)
{
    /**
     * @memberof mem_t
     * @brief 釋放動態的物件。
     *
     * @param self Object instance.
     */
    if( self )
    {
        mem_deinit(self);
        free(self);
    }
}
//------------------------------------------------------------------------------
void mem_release_s(mem_t** RESTRICT self)
{
    /**
     * @memberof mem_t
     * @brief 釋放動態的物件，並重設物件指標為NULL。
     *
     * @param self Reference of the object instance.
     */
    if( self )
    {
        mem_release(*self);
        *self = NULL;
    }
}
//------------------------------------------------------------------------------
void mem_set_zeros(mem_t* RESTRICT self)
{
    /**
     * @memberof mem_t
     * @brief 將緩衝區的資料全設定為零。
     *
     * @param self Object instance.
     */
    if( self ) memset(self->buf, 0, self->size);
}
//------------------------------------------------------------------------------
bool mem_resize(mem_t* RESTRICT self, size_t size)
{
    /**
     * @memberof mem_t
     * @brief 變更緩衝區大小。
     *
     * @param self Object instance.
     * @param size New size of the data buffer.
     * @return TRUE if succeed; and FALSE if failed.
     *
     * @remarks Data in the buffer will be saved if the new size is great equal than the old;
     *          and will be truncated if the new size is less than the old.
     */
    if( !self ) return false;

    if( self->size_total >= size )
    {
        self->size = size;
    }
    else
    {
        size_t  newsize;
        byte_t *newbuf;

        newsize = memobj_calc_recommended_size(size);
        if( newsize < MEMOJB_BLOCKSZ_LARGE ) newsize <<= 1;

        newbuf = realloc(self->buf, newsize);
        if( !newbuf ) return false;

        self->size_total = newsize;
        self->size       = size;
        self->buf        = newbuf;
    }
    assert( self->size_total >= self->size );

    return true;
}
//------------------------------------------------------------------------------
bool mem_import(mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof mem_t
     * @brief 將一段資料內容設定至物件緩衝區內。
     *
     * @param self   Object instance.
     * @param buffer Data to import from.
     * @param size   Size of data to import.
     * @return TRUE if succeed; and FALSE if failed.
     */
    if( !self || !buffer ) return false;

    if( !mem_resize(self, size) ) return false;
    memcpy(self->buf, buffer, size);

    return true;
}
//------------------------------------------------------------------------------
bool mem_append(mem_t* RESTRICT self, const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof mem_t
     * @brief 將一段資料添加至物件緩衝區原有資料的後面。
     *
     * @param self   Object instance.
     * @param buffer Data to append.
     * @param size   Size of data to append.
     * @return TRUE if succeed; and FALSE if failed.
     */
    size_t sizeold;

    if( !self || !buffer ) return false;

    sizeold = self->size;
    if( !mem_resize(self, sizeold+size) ) return false;
    memcpy(self->buf+sizeold, buffer, size);

    return true;
}
//------------------------------------------------------------------------------
void mem_move_from(mem_t* RESTRICT self, mem_t* RESTRICT src)
{
    /**
     * @memberof mem_t
     * @brief 物件資料搬移。
     *
     * @param self Object instance.
     * @param src  Data to move from.
     * @return TRUE if succeed; and FALSE if failed.
     */
    if( !self || !src ) return;

    free(self->buf);

    self->size_total = src->size_total;
    self->size       = src->size;
    self->buf        = src->buf;
    src->size_total  = memobj_calc_recommended_size(0);
    src->size        = 0;
    src->buf         = malloc(src->size_total);
    assert( src->buf );
}
//------------------------------------------------------------------------------
void mem_pop_front(mem_t* RESTRICT self, size_t popsz)
{
    /**
     * @memberof mem_t
     * @brief 將緩衝區頭部的一段資料移除，並將其他資料前移。
     *
     * @param self  Object instance.
     * @param popsz Size of data to pop.
     */
    if( !self ) return;

    if( popsz < self->size )
    {
        byte_t *dest = self->buf;
        byte_t *src  = self->buf + popsz;
        size_t  i;

        for(i = self->size - popsz;
            i--;
            )
        {
            *dest++ = *src++;
        }
        self->size -= popsz;
    }
    else
    {
        self->size = 0;
    }
}
//------------------------------------------------------------------------------
int mem_compare(const mem_t* RESTRICT self, const mem_t* RESTRICT tar)
{
    /**
     * @memberof mem_t
     * @brief 比較兩個緩衝區的資料。
     *
     * @param self Object instance.
     * @param tar  An other object to be compared.
     * @retval negative The data size of current object is shorter than the other, or
                        the first byte that does not match in both data blocks
     *                  has a lower value in the current object than the other.
     * @retval zero     The contents of both object are equal.
     * @retval positive The data size of current object is larger than the other, or
     *                  the first byte that does not match in both data blocks
     *                  has a greater value in the current object than the other.
     */
    if( !self ) return -1;
    if( !tar ) return 1;

    if( self->size != tar->size ) return self->size < tar->size ? -1 : 1;
    return memcmp(self->buf, tar->buf, self->size);
}
//------------------------------------------------------------------------------
const void* mem_find(const mem_t* RESTRICT self, const mem_t* RESTRICT pattern)
{
    /**
     * @memberof mem_t
     * @see ::memfind
     */
    return ( self && pattern )?
           ( memfind(self->buf, self->size, pattern->buf, pattern->size) ):
           ( NULL );
}
//------------------------------------------------------------------------------
const void* mem_rfind(const mem_t* RESTRICT self, const mem_t* RESTRICT pattern)
{
    /**
     * @memberof mem_t
     * @see ::memrfind
     */
    return ( self && pattern )?
           ( memrfind(self->buf, self->size, pattern->buf, pattern->size) ):
           ( NULL );
}
//------------------------------------------------------------------------------
unsigned mem_find_count(const mem_t* RESTRICT self, const mem_t* RESTRICT pattern)
{
    /**
     * @memberof mem_t
     * @see ::memfindcount
     */
    return ( self && pattern )?
           ( memfindcount(self->buf, self->size, pattern->buf, pattern->size) ):
           ( 0 );
}
//------------------------------------------------------------------------------
bool mem_find_replace(mem_t* RESTRICT self, const mem_t* RESTRICT pattern, const mem_t* RESTRICT target)
{
    /**
     * @memberof mem_t
     * @brief 搜尋一段記憶體中的特定資料，並將之取代為另一個指定的資料。
     *
     * @param self    Object instance.
     * @param pattern Search pattern.
     * @param target  Data to replace search pattern.
     * @return TRTUE if succeed; and FALSE if failed.
     */
    if( !self || !pattern || !target ) return false;

    mem_t src;
    mem_init(&src, 0);

    bool res = false;
    do
    {
        if( !mem_import(&src, self->buf, self->size) ) break;

        size_t sizeneed = memfindreplace(NULL,
                                         0,
                                         src.buf,
                                         src.size,
                                         pattern->buf,
                                         pattern->size,
                                         target->buf,
                                         target->size);
        if( !sizeneed || !mem_resize(self, sizeneed) ) break;

        size_t sizefinal = memfindreplace(self->buf,
                                          self->size,
                                          src.buf,
                                          src.size,
                                          pattern->buf,
                                          pattern->size,
                                          target->buf,
                                          target->size);
        if( !sizefinal || !mem_resize(self, sizefinal) ) break;

        res = true;
    } while(false);

    mem_deinit(&src);

    return res;
}
//------------------------------------------------------------------------------
bool mem_save_file(const mem_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof mem_t
     * @brief 將緩衝區內容存入檔案。
     *
     * @param self     Object instance.
     * @param filename Name of file to save data.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE *file    = NULL;
    bool  succeed = false;

    if( !self || !filename ) return false;

    do
    {
        file = file_open_writable(filename);
        if( !file ) break;

        if( !file_write_binary(file, self->buf, self->size) ) break;

        succeed = true;
    } while(false);

    if( file ) fclose(file);

    return succeed;
}
//------------------------------------------------------------------------------
bool mem_load_file(mem_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof mem_t
     * @brief 將檔案內容讀入至緩衝區。
     *
     * @param self     Object instance.
     * @param filename Name of file to load data.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE  *file    = NULL;
    mem_t *memtemp = NULL;
    bool   succeed = false;

    if( !self || !filename ) return false;

    do
    {
        long   filesz;
        size_t recsz;

        file = file_open_readonly(filename);
        if( !file ) break;

        filesz = file_get_size(file);
        if( filesz < 0 ) break;

        memtemp = mem_create(filesz);
        if( !memtemp ) break;

        recsz = file_read_binary(file, memtemp->buf, memtemp->size);
        if( recsz != (size_t)filesz ) break;

        succeed = true;
    } while(false);

    if( succeed ) mem_move_from(self, memtemp);

    if( file ) fclose(file);
    mem_release(memtemp);

    return succeed;
}
//------------------------------------------------------------------------------
bool mem_save_text(const mem_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof mem_t
     * @brief 將緩衝區內容存入為文字檔案，通常在除錯與人工監視記憶體內容時使用。
     *
     * @param self     Object instance.
     * @param filename Name of file to save information.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE *file = NULL;

    if( !self || !filename ) return false;

    file = file_open_writable_text(filename);
    if( !file ) return false;

    file_write_text(file, self->buf, self->size);

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------
//---- Memory Object (Fixed Buffer) --------------------------------------------
//------------------------------------------------------------------------------
static
size_t memfx_get_userbuf_sizemax(const memfx_t* RESTRICT self)
{
    return self->size_total - sizeof(memfx_header_t);
}
//------------------------------------------------------------------------------
memfx_t* memfx_init(byte_t* RESTRICT buffer, size_t bufsz, size_t initsz)
{
    /**
     * @memberof memfx_t
     * @brief Constructor.
     *
     * @param buffer 傳入一個要給 @ref memfx_t 物件使用的資料緩衝區，這個緩衝區在 @ref memfx_t 物件的整個生命週期中都必須為有效。
     *               在初始化成功後，緩衝區的頭部將會被用於存放物件管理相關資訊，並將其餘部份做為使用者資料緩衝空間。
     * @param bufsz  傳入 buffer 的大小。
     * @param initsz 傳入 memfx 物件初始的使用者緩衝空間大小，此僅為初始化使用，該數值在物件後續的使用操作中仍可被更改。
     *               若傳入之數值大於緩衝區可用做使用者緩衝空間的最大值，則物件將以實際可用的最大值做為設定。
     * @return 初始化成功時傳回 @ref memfx_t 類型的物件指標，該指標事實上就是使用者傳入之 buffer 位址；
     *         而在初始化失敗時傳回 NULL。
     *         初始化失敗只有一個原因，就是使用者輸入的緩衝區過小，以至於不能容納基本需要的標頭資訊。
     */
    memfx_t *self = (memfx_t*) buffer;

    if( !buffer || bufsz < sizeof(memfx_header_t) ) return NULL;

    self->size_total = bufsz;
    self->size       = MIN( bufsz - sizeof(memfx_header_t), initsz );

    return self;
}
//------------------------------------------------------------------------------
void memfx_set_zeros(memfx_t* RESTRICT self)
{
    /**
     * @memberof memfx_t
     * @brief 將當前使用者空間內的資料全部清為零。
     *
     * @param self Object instance.
     */
    if( !self ) return;

    memset(self->buf, 0, self->size);
}
//------------------------------------------------------------------------------
bool memfx_resize(memfx_t* RESTRICT self, size_t size)
{
    /**
     * @memberof memfx_t
     * @brief 變更使用者緩衝空間大小。
     *
     * @param self Object instance.
     * @param size The new size of data buffer.
     * @return TRUE if succeed; and FALSE if failed.
     */
    if( !self ) return false;
    if( size > memfx_get_userbuf_sizemax(self) ) return false;

    self->size = size;

    return true;
}
//------------------------------------------------------------------------------
bool memfx_import(memfx_t* RESTRICT self, const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof memfx_t
     * @brief 將一段資料內容設定至物件緩衝區內。
     *
     * @param self   Object instance.
     * @param buffer Data to import from.
     * @param size   Size of data to import from.
     * @return TRUE if succeed; and FALSE if failed.
     */
    if( !self || !buffer ) return false;
    if( size > memfx_get_userbuf_sizemax(self) ) return false;

    self->size = size;
    memcpy(self->buf, buffer, size);

    return true;
}
//------------------------------------------------------------------------------
bool memfx_append(memfx_t* RESTRICT self, const void* RESTRICT buffer, size_t size)
{
    /**
     * @memberof memfx_t
     * @brief 將一段資料添加至物件緩衝區原有資料的後面。
     *
     * @param self   Object instance.
     * @param buffer Data to append.
     * @param size   Size of data to append.
     * @return TRUE if succeed; and FALSE if failed.
     */
    if( !self || !buffer ) return false;
    if( self->size + size > memfx_get_userbuf_sizemax(self) ) return false;
    if(              size > memfx_get_userbuf_sizemax(self) ) return false;  // Check for security reason

    memcpy( self->buf + self->size, buffer, size );
    self->size += size;

    return true;
}
//------------------------------------------------------------------------------
void memfx_pop_front(memfx_t* RESTRICT self, size_t popsz)
{
    /**
     * @memberof memfx_t
     * @brief 將緩衝區頭部的一段資料移除，並將其他資料前移。
     *
     * @param self  Object instance.
     * @param popsz Size of data to pop.
     */
    if( !self ) return;

    if( popsz < self->size )
    {
        byte_t *dest = self->buf;
        byte_t *src  = self->buf + popsz;
        size_t  i;

        for(i = self->size - popsz;
            i--;
            )
        {
            *dest++ = *src++;
        }
        self->size -= popsz;
    }
    else
    {
        self->size = 0;
    }
}
//------------------------------------------------------------------------------
int memfx_compare(const memfx_t* RESTRICT self, const memfx_t* RESTRICT tar)
{
    /**
     * @memberof mem_t
     * @brief 比較兩個緩衝區的資料。
     *
     * @param self Object instance.
     * @param tar  An other object to be compared.
     * @retval negative The data size of current object is shorter than the other, or
                        the first byte that does not match in both data blocks
     *                  has a lower value in the current object than the other.
     * @retval zero     The contents of both object are equal.
     * @retval positive The data size of current object is larger than the other, or
     *                  the first byte that does not match in both data blocks
     *                  has a greater value in the current object than the other.
     */
    if( !self ) return -1;
    if( !tar ) return 1;

    if( self->size != tar->size ) return self->size < tar->size ? -1 : 1;
    return memcmp(self->buf, tar->buf, self->size);
}
//------------------------------------------------------------------------------
const void* memfx_find(const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern)
{
    /**
     * @memberof memfx_t
     * @see ::memfind
     */
    return ( self && pattern )?
           ( memfind(self->buf, self->size, pattern->buf, pattern->size) ):
           ( NULL );
}
//------------------------------------------------------------------------------
const void* memfx_rfind(const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern)
{
    /**
     * @memberof memfx_t
     * @see ::memrfind
     */
    return ( self && pattern )?
           ( memrfind(self->buf, self->size, pattern->buf, pattern->size) ):
           ( NULL );
}
//------------------------------------------------------------------------------
unsigned memfx_find_count(const memfx_t* RESTRICT self, const memfx_t* RESTRICT pattern)
{
    /**
     * @memberof memfx_t
     * @see ::memfindcount
     */
    return ( self && pattern )?
           ( memfindcount(self->buf, self->size, pattern->buf, pattern->size) ):
           ( 0 );
}
//------------------------------------------------------------------------------
bool memfx_save_file(const memfx_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof memfx_t
     * @brief 將緩衝區內容存入檔案。
     *
     * @param self     Object instance.
     * @param filename Name of file to save information.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE *file    = NULL;
    bool  succeed = false;

    if( !self || !filename ) return false;

    do
    {
        file = file_open_writable(filename);
        if( !file ) break;

        if( !file_write_binary(file, self->buf, self->size) ) break;

        succeed = true;
    } while(false);

    if( file ) fclose(file);

    return succeed;
}
//------------------------------------------------------------------------------
bool memfx_load_file(memfx_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof memfx_t
     * @brief 將檔案內容讀入至緩衝區
     *
     * @param self     Object instance.
     * @param filename Name of file to load data.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE *file    = NULL;
    bool  succeed = false;

    if( !self || !filename ) return false;

    do
    {
        long   filesz;
        size_t recsz;

        file = file_open_readonly(filename);
        if( !file ) break;

        filesz = file_get_size(file);
        if( filesz < 0 ) break;

        if( filesz > memfx_get_userbuf_sizemax(self) ) break;

        self->size = recsz = file_read_binary(file, self->buf, filesz);
        if( recsz != (size_t)filesz ) break;

        succeed = true;
    } while(false);

    if( file ) fclose(file);

    return succeed;
}
//------------------------------------------------------------------------------
bool memfx_save_text(const memfx_t* RESTRICT self, const char* RESTRICT filename)
{
    /**
     * @memberof memfx_t
     * @brief 將緩衝區內容存入為文字檔案，通常在除錯與人工監視記憶體內容時使用。
     *
     * @param self     Object instance.
     * @param filename Name of file to save information.
     * @return TRUE if succeed; and FALSE if failed.
     */
    FILE *file = NULL;

    if( !self || !filename ) return false;

    file = file_open_writable_text(filename);
    if( !file ) return false;

    file_write_text(file, self->buf, self->size);

    fclose(file);
    return true;
}
//------------------------------------------------------------------------------

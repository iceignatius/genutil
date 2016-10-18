#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "file.h"

int main(void)
{
    char buf[1024] = {0};

    // Path information inquiry.
    {
        printf("[Path information inquiry test]\n");

        printf("working directory   : %s\n", file_get_working_dir());
        printf("Home position       : %s\n", file_get_home_dir());
        printf("Temporary directory : %s\n", file_get_temp_dir());

        printf("\n");
    }

    // File name extension test.
    {
        file_get_file_ext(buf, sizeof(buf), "file.name.ext");
        assert( 0 == strcmp(buf, ".ext") );

        file_get_file_ext(buf, sizeof(buf), "file_name");
        assert( 0 == strcmp(buf, "") );

        file_set_file_ext(buf, sizeof(buf), "file.name.old", ".new");
        assert( 0 == strcmp(buf, "file.name.new") );

        file_set_file_ext(buf, sizeof(buf), "file_name", ".new");
        assert( 0 == strcmp(buf, "file_name.new") );
    }

    // File name total extension test.
    {
        file_get_file_ext_all(buf, sizeof(buf), "some.path.to/file.ext.total");
        assert( 0 == strcmp(buf, ".ext.total") );

        file_get_file_ext_all(buf, sizeof(buf), "some.path.to/file");
        assert( 0 == strcmp(buf, "") );

        file_set_file_ext_all(buf, sizeof(buf), "some.path.to/file.ext.total", ".new.ext");
        assert( 0 == strcmp(buf, "some.path.to/file.new.ext") );

        file_set_file_ext_all(buf, sizeof(buf), "some.path.to/file", ".new.ext");
        assert( 0 == strcmp(buf, "some.path.to/file.new.ext") );
    }

    // File name extraction test.
    {
        file_get_file_name(buf, sizeof(buf), "file.ext");
        assert( 0 == strcmp(buf, "file.ext") );

        file_get_file_name(buf, sizeof(buf), "/path/to/file.ext");
        assert( 0 == strcmp(buf, "file.ext") );
    }

    // File directory extraction test.
    {
        file_get_file_dir(buf, sizeof(buf), "/path/to/file/");
        assert( 0 == strcmp(buf, "/path/to/file") );

        file_get_file_dir(buf, sizeof(buf), "/path/to/file");
        assert( 0 == strcmp(buf, "/path/to") );

        file_get_file_dir(buf, sizeof(buf), "/file");
        assert( 0 == strcmp(buf, "") );

        file_get_file_dir(buf, sizeof(buf), "/");
        assert( 0 == strcmp(buf, "") );

        file_get_file_dir(buf, sizeof(buf), "file");
        assert( 0 == strcmp(buf, "") );
    }

    // File path extraction test.
    {
        file_get_file_path(buf, sizeof(buf), "/path/to/file/");
        assert( 0 == strcmp(buf, "/path/to/") );

        file_get_file_path(buf, sizeof(buf), "/path/to/file");
        assert( 0 == strcmp(buf, "/path/to/") );

        file_get_file_path(buf, sizeof(buf), "/file");
        assert( 0 == strcmp(buf, "/") );

        file_get_file_path(buf, sizeof(buf), "/");
        assert( 0 == strcmp(buf, "/") );

        file_get_file_path(buf, sizeof(buf), "file");
        assert( 0 == strcmp(buf, "") );

        file_get_file_path(buf, sizeof(buf), "C:/");
        assert( 0 == strcmp(buf, "C:/") );
    }

    // Absolute path test.
    {
        printf("[Absolute path test]\n");

        const char *dir;

        dir = ".././previous_path/";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        dir = ".././previous_dir";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        dir = "~/./under_home";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        dir = "/.././under_root";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        dir = "C:/.././under_root";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        dir = "";
        file_get_abs_path(buf, sizeof(buf), dir);
        printf("\"%s\" \t-> \"%s\"\n", dir, buf);

        printf("\n");
    }

    // Relative path test.
    {
        file_get_rel_path(buf, sizeof(buf), "/test/dir/file", "/test/path/base");
        assert( 0 == strcmp(buf, "../../dir/file") );

        file_get_rel_path(buf, sizeof(buf), "/test/path/base", "/test/path/base");
        assert( 0 == strcmp(buf, ".") );

        file_get_rel_path(buf, sizeof(buf), "/test/path/base/", "/test/path/base");
        assert( 0 == strcmp(buf, "./") );

        file_get_rel_path(buf, sizeof(buf), "/test/path", "/test/path/base");
        assert( 0 == strcmp(buf, "..") );

        file_get_rel_path(buf, sizeof(buf), "/test/path/", "/test/path/base");
        assert( 0 == strcmp(buf, "../") );

        file_get_rel_path(buf, sizeof(buf), "/test/path/base/dir/file", "/test/path/base");
        assert( 0 == strcmp(buf, "dir/file") );

        file_get_rel_path(buf, sizeof(buf), "/test/path/base/dir/file", "/test/path/base/");
        assert( 0 == strcmp(buf, "dir/file") );

        file_get_rel_path(buf, sizeof(buf), "C:/another/path/file", "/test/path/base/");
        assert( 0 == strcmp(buf, "C:/another/path/file") );

        file_get_rel_path(buf, sizeof(buf), "", "/test/path/base");
        assert( 0 == strcmp(buf, "") );

    }

    // Directory create and remove test.
    {
        static const char dirname1[] = "test_directory";
        static const char dirname2[] = "test_directory/sub_directory";

        // Make sure that the test targets does not existed.
        file_remove_dir(dirname2);
        file_remove_dir(dirname1);

        assert( !file_is_dir_existed(dirname1) );
        assert( !file_is_file_existed(dirname1) );
        assert( !file_is_dir_existed(dirname2) );
        assert( !file_is_file_existed(dirname2) );

        assert(  file_create_dir(dirname1) );
        assert(  file_is_dir_existed(dirname1) );
        assert( !file_is_file_existed(dirname1) );

        assert(  file_create_dir(dirname2) );
        assert(  file_is_dir_existed(dirname2) );
        assert( !file_is_file_existed(dirname2) );

        assert( !file_is_dir_empty(dirname1) );
        assert(  file_is_dir_empty(dirname2) );

        assert(  file_remove_dir(dirname2) );
        assert( !file_is_dir_existed(dirname2) );
        assert( !file_is_file_existed(dirname2) );

        assert(  file_remove_dir(dirname1) );
        assert( !file_is_dir_existed(dirname1) );
        assert( !file_is_file_existed(dirname1) );
    }

    // Directory with slash test.
    {
        static const char dirname[] = "test_directory/";

        // Make sure that the test targets does not existed.
        file_remove_dir(dirname);

        assert( !file_is_dir_existed(dirname) );
        assert( !file_is_file_existed(dirname) );

        assert(  file_create_dir(dirname) );
        assert(  file_is_dir_existed(dirname) );
        assert( !file_is_file_existed(dirname) );

        assert( file_is_dir_empty(dirname) );

        assert(  file_remove_dir(dirname) );
        assert( !file_is_dir_existed(dirname) );
        assert( !file_is_file_existed(dirname) );
    }

    // Recursive create and remove test.
    {
        static const char pathall [] = "test_directory/aa/bb/cc/dd/final_directory/";
        static const char pathroot[] = "test_directory/";

        // Make sure that the test targets does not existed.
        file_remove_recursive(pathroot);

        assert( !file_is_dir_existed(pathall) );
        assert( !file_is_file_existed(pathall) );
        assert( !file_is_dir_existed(pathroot) );
        assert( !file_is_file_existed(pathroot) );

        assert(  file_create_path(pathall) );
        assert(  file_is_dir_existed(pathall) );
        assert( !file_is_file_existed(pathall) );

        assert(  file_remove_recursive(pathroot) );
        assert( !file_is_dir_existed(pathall) );
        assert( !file_is_file_existed(pathall) );
    }

    // Files enumeration.
    {
        file_list_t *list;

        printf("Enumerate files - name only\n");
        assert(( list = file_list_files("../", FILE_ENUM_NAMEONLY) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");

        printf("Enumerate files - directory slash\n");
        assert(( list = file_list_files("../", FILE_ENUM_DIRSLASH) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");

        printf("Enumerate files - recursively\n");
        assert(( list = file_list_files("../", FILE_ENUM_DIRSLASH|FILE_ENUM_RECURSIVE) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");

        printf("Enumerate files - have prefix\n");
        assert(( list = file_list_files("../", FILE_ENUM_DIRSLASH|FILE_ENUM_RECURSIVE|FILE_ENUM_PREFIX) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");

        printf("Enumerate files - files only\n");
        assert(( list = file_list_files("../", FILE_ENUM_DIRSLASH|FILE_ENUM_RECURSIVE|FILE_ENUM_PREFIX|FILE_ENUM_FILEONLY) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");

        printf("Enumerate files - directory only\n");
        assert(( list = file_list_files("../", FILE_ENUM_DIRSLASH|FILE_ENUM_RECURSIVE|FILE_ENUM_PREFIX|FILE_ENUM_DIRONLY) ));
        for(unsigned i=0; i<list->count; ++i)
            printf("  [%s]\n", list->names[i]);
        printf("\n");
    }

    return 0;
}

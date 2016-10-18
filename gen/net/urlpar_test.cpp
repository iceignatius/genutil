#include <assert.h>
#include <string.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "urlpar.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

int main()
{
    // Extract URL parts - 1
    {
        static const char url[] = "scheme://user:password@domain:port/path?query_string#fragment_id";
        char buf[256];

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_sceme(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "scheme") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_user(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "user") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_pass(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "password") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_host(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "domain") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_port(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "port") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_path(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "path") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_query(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "query_string") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_fragid(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "fragment_id") );
    }

    // Extract URL parts - 2
    {
        static const char url[] = "scheme://domain:port/path?query_string";
        char buf[256];

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_sceme(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "scheme") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_user(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_pass(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_host(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "domain") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_port(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "port") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_path(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "path") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_query(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "query_string") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_fragid(buf, sizeof(buf), url) );
    }

    // Extract URL parts - 3
    {
        static const char url[] = "scheme://domain/path";
        char buf[256];

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_sceme(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "scheme") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_user(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_pass(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_host(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "domain") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_port(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_path(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "path") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_query(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_fragid(buf, sizeof(buf), url) );
    }

    // Extract URL parts - 4
    {
        static const char url[] = "domain:port";
        char buf[256];

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_sceme(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_user(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_pass(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_host(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "domain") );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_port(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "port") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_path(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_query(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_fragid(buf, sizeof(buf), url) );
    }

    // Extract URL parts - 5
    {
        static const char url[] = "domain";
        char buf[256];

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_sceme(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_user(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_pass(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( urlpar_extract_host(buf, sizeof(buf), url) );
        assert( 0 == strcmp(buf, "domain") );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_port(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_path(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_query(buf, sizeof(buf), url) );

        memset(buf, -1, sizeof(buf));
        assert( !urlpar_extract_fragid(buf, sizeof(buf), url) );
    }

    // Port number analysis
    {
        assert( 123 == urlpar_get_port("http://domain_name:123") );
        assert( 456 == urlpar_get_port("http://domain_name:456") );
        assert(  80 == urlpar_get_port("http://domain_name") );
        assert( 443 == urlpar_get_port("https://domain_name") );
        assert( 465 == urlpar_get_port("smtps://domain_name") );
        assert(   0 == urlpar_get_port("domain_name") );
    }

    return 0;
}

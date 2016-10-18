#include <assert.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "iptype.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

int main()
{
    const ipv4_t      ip4val       = ipv4_from_digits(192,168,1,255);
    const char* const ip4str       = "192.168.1.255";

    const ipv6_t      ip6val       = ipv6_from_digits(0,0,0,0,0,0xFFFF,0xC0A8,0x01FF);  // 192.168.1.255
    const char* const ip6str_input = "::ffff:192.168.1.255";
    const char* const ip6str_full  = "0000:0000:0000:0000:0000:FFFF:C0A8:01FF";
    const char* const ip6str_short = "::FFFF:C0A8:1FF";

    const macaddr_t   macaddr      = macaddr_from_digits(0x11,0x22,0xCA,0xFB,0x44,0x55);
    const char       *macstr       = "11:22:CA:FB:44:55";

    // ==== IPv4 ====

    // IP digits
    {
        assert( ip4val.dig[0] == 192 );
        assert( ip4val.dig[1] == 168 );
        assert( ip4val.dig[2] ==   1 );
        assert( ip4val.dig[3] == 255 );
    }
    // IP integer and string conversion
    {
        assert( ipv4_to_int(ip4val) == 0xC0A801FF );
        assert( ipv4_from_int(0xC0A801FF).val == ip4val.val );
        assert( 0 == IPv4ToStr(ip4val).compare(ip4str) );
        assert( ipv4_from_str(ip4str).val == ip4val.val );
    }
    // Broadcast IP calculation
    {
        ipv4_t ip          = ipv4_from_digits(192,168,  1,  1);
        ipv4_t mask        = ipv4_from_digits(255,255,255,  0);
        ipv4_t broadcastip = ipv4_from_digits(192,168,  1,255);
        assert( ipv4_calc_broadcast(ip, mask).val == broadcastip.val );
    }

    // ==== IPv6 ====

    // IP digits
    {
        assert( ipv6_get_digit(ip6val, 0) == 0      );
        assert( ipv6_get_digit(ip6val, 1) == 0      );
        assert( ipv6_get_digit(ip6val, 2) == 0      );
        assert( ipv6_get_digit(ip6val, 3) == 0      );
        assert( ipv6_get_digit(ip6val, 4) == 0      );
        assert( ipv6_get_digit(ip6val, 5) == 0xFFFF );
        assert( ipv6_get_digit(ip6val, 6) == 0xC0A8 );
        assert( ipv6_get_digit(ip6val, 7) == 0x01FF );
    }
    // IP type and string conversion
    {
        assert( 0 == IPv6ToStrFull(ip6val).compare(ip6str_full) );
        assert( 0 == IPv6ToStrShort(ipv6_from_digits(1,2,3,4,5,6,7,8)).compare("1:2:3:4:5:6:7:8") );
        assert( 0 == IPv6ToStrShort(ipv6_from_digits(1,2,0,0,0,0,7,8)).compare("1:2::7:8"       ) );
        assert( 0 == IPv6ToStrShort(ipv6_from_digits(0,0,0,0,5,6,7,8)).compare("::5:6:7:8"      ) );
        assert( 0 == IPv6ToStrShort(ipv6_from_digits(1,2,3,4,0,0,0,0)).compare("1:2:3:4::"      ) );
        assert( 0 == IPv6ToStrShort(ip6val).compare(ip6str_short) );
        assert( ipv6_is_equal( ipv6_from_str("1:2:3:4:5:6:7:8"), ipv6_from_digits(1,2,3,4,5,6,7,8) ) );
        assert( ipv6_is_equal( ipv6_from_str("1:2::7:8"       ), ipv6_from_digits(1,2,0,0,0,0,7,8) ) );
        assert( ipv6_is_equal( ipv6_from_str("::5:6:7:8"      ), ipv6_from_digits(0,0,0,0,5,6,7,8) ) );
        assert( ipv6_is_equal( ipv6_from_str("1:2:3:4::"      ), ipv6_from_digits(1,2,3,4,0,0,0,0) ) );
        assert( ipv6_is_equal( ipv6_from_str(ip6str_input), ip6val ) );
        assert( ipv6_is_equal( ipv6_from_str(ip4str)      , ip6val ) );
    }
    // IP version conversion
    {
        assert( ipv6_is_equal( ipv6_from_ipv4(ip4val), ip6val ) );
    }

    // ==== MAC Address ====

    // MAC digits
    {
        assert( macaddr.dig[0] == 0x11 );
        assert( macaddr.dig[1] == 0x22 );
        assert( macaddr.dig[2] == 0xCA );
        assert( macaddr.dig[3] == 0xFB );
        assert( macaddr.dig[4] == 0x44 );
        assert( macaddr.dig[5] == 0x55 );
    }
    // MAC type and string conversion
    {
        assert( 0 == MacAddrToStr(macaddr).compare(macstr) );
        assert( macaddr_is_equal( macaddr_from_str(macstr), macaddr ) );
    }

    return 0;
}

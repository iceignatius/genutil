/**
 * @file
 * @brief     IP type.
 * @details   Translation tools and definition about IP type.
 * @author    王文佑
 * @date      2014.03.12
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_IPTYPE_H_
#define _GEN_NET_IPTYPE_H_

#ifdef __cplusplus
#include <string>
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../static_assert.h"
#include "../inline.h"

/**
 * @name IPv4
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/// IP type - IPv4
#pragma pack(push,1)
typedef union ipv4_t
{
    uint8_t  dig[4];  ///< 4 Digits.
    uint32_t val;     ///< 32-bits integer value.
} ipv4_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(ipv4_t) == 4 ); ///< @private

INLINE ipv4_t ipv4_from_digits(int d0, int d1, int d2, int d3)
{
    /**
     * 指定個別的數字來建立一個 IP。
     *
     * @param d0, d1, d2, d3 由頭到尾的 4 個數字，值域為 0 至 255。
     * @return 由傳入參數所組合建立的 IP。
     */
    ipv4_t ip;
    ip.dig[0] = d0;
    ip.dig[1] = d1;
    ip.dig[2] = d2;
    ip.dig[3] = d3;
    return ip;
}

INLINE ipv4_t ipv4_from_int(uint32_t val)
{
    /**
     * 指定一個等效的 32 位元整數來建立一個 IP。
     *
     * @param val 傳入的 32 位元整數。
     * @return 由傳入參數所組合建立的 IP。
     */
    return ipv4_from_digits( val>>24, (val>>16)&0xFF, (val>>8)&0xFF, val&0xFF );
}

INLINE uint32_t ipv4_to_int(ipv4_t ip)
{
    /**
     * 取得 IP 的等效 32 位元整數。
     *
     * @param ip 傳入的 IP。
     * @return 所傳入 IP 之等效 32 位元整數。
     */
    return ( ip.dig[0]<<24 )|( ip.dig[1]<<16 )|( ip.dig[2]<<8 )|( ip.dig[3] );
}

size_t ipv4_to_str(char* dest, size_t destsz, ipv4_t ip);

ipv4_t ipv4_from_str(const char* str);

INLINE ipv4_t ipv4_calc_broadcast(ipv4_t ip, ipv4_t mask)
{
    /**
     * 計算廣播位址。
     *
     * @param ip   位於網域內的任意 IP，這個位址上沒有對應的裝置也沒關係，
     *             因為用以計算的是網域位址。
     * @param mask 該網域的通到遮罩。
     * @return 傳回該網域內的廣播位址。
     */
    ipv4_t res;
    res.val = ip.val | ~mask.val;
    return res;
}

#ifdef __cplusplus
}  // extern "C"
#endif

// C++ Wrapper
#ifdef __cplusplus

inline std::string IPv4ToStr(const ipv4_t& IP)
{
    /// @see ::ipv4_to_str
    std::string res;
    res.resize( ipv4_to_str(NULL, 0, IP) - 1 );
    ipv4_to_str((char*)res.c_str(), res.length()+1, IP);
    return res;
}

#endif

/**
 * @}
 */

/**
 * @name IPv6
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/// IPv6 digit
#pragma pack(push,1)
typedef struct ipv6_dig_t
{
    uint8_t high;  ///< High 8-bits part of the digit.
    uint8_t low;   ///< Low 8-bits part of the digit.
} ipv6_dig_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(ipv6_dig_t) == 2 ); ///< @private

/// IP type - IPv6
#pragma pack(push,1)
typedef union ipv6_t
{
    ipv6_dig_t dig  [8];  ///< 8 digits.
    uint64_t   val64[2];
} ipv6_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(ipv6_t) == 16 ); ///< @private

INLINE bool ipv6_is_equal(ipv6_t ip1, ipv6_t ip2)
{
    /**
     * 比對兩個 IP 是否相等。
     *
     * @param ip1, ip2 欲比較的兩個 IP。
     * @return 傳回兩 IP 是否相等的邏輯。
     */
    return ( ip1.val64[0] == ip2.val64[0] )&&( ip1.val64[1] == ip2.val64[1] );
}

INLINE ipv6_t ipv6_from_digits(int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
{
    /**
     * 指定個別的數字來建立一個 IP。
     *
     * @param d0, d1, d2, d3, d4, d5, d6, d7 由頭到尾的 8 個數字，值域為 0 至 65535。
     * @return 由傳入參數所組合建立的 IP。
     */
    ipv6_t ip;
    ip.dig[0].high = d0 >> 8;
    ip.dig[0].low  = d0 &  0xFF;
    ip.dig[1].high = d1 >> 8;
    ip.dig[1].low  = d1 &  0xFF;
    ip.dig[2].high = d2 >> 8;
    ip.dig[2].low  = d2 &  0xFF;
    ip.dig[3].high = d3 >> 8;
    ip.dig[3].low  = d3 &  0xFF;
    ip.dig[4].high = d4 >> 8;
    ip.dig[4].low  = d4 &  0xFF;
    ip.dig[5].high = d5 >> 8;
    ip.dig[5].low  = d5 &  0xFF;
    ip.dig[6].high = d6 >> 8;
    ip.dig[6].low  = d6 &  0xFF;
    ip.dig[7].high = d7 >> 8;
    ip.dig[7].low  = d7 &  0xFF;
    return ip;
}

INLINE int ipv6_get_digit(ipv6_t ip, int ind)
{
    /**
     * 取得 IP 內的個別數字。
     *
     * @param ip  欲做查詢的 IP。
     * @param ind 欲取得的 IP 數字之引數，值域為 0 至 7。
     * @return 傳回 IP 指定引數下的數字。
     */
    ind &= 0x07;
    return ( ip.dig[ind].high << 8 ) | ip.dig[ind].low;
}

size_t ipv6_to_str_full(char* dest, size_t destsz, ipv6_t ip);

size_t ipv6_to_str_short(char* dest, size_t destsz, ipv6_t ip);

ipv6_t ipv6_from_str(const char* str);

INLINE size_t ipv6_to_str(char* dest, size_t destsz, ipv6_t ip)
{
    /// Equivalence to ::ipv6_to_str_short with a short name.
    return ipv6_to_str_short(dest, destsz, ip);
}

INLINE ipv6_t ipv6_from_ipv4(ipv4_t ip)
{
    /**
     * 從 IPv4 版本 IP 建立一個等效的 IPv6 版 IP。
     *
     * @param ip 傳入一個 IPv4 版本 IP。
     * @return 傳回等效的 IPv6 版本 IP。
     */
    return ipv6_from_digits(0,0,0,0,0,
                            0xFFFF,
                            ( ip.dig[0] << 8 ) | ip.dig[1],
                            ( ip.dig[2] << 8 ) | ip.dig[3]);
}

#ifdef __cplusplus
}  // extern "C"
#endif

// C++ Wrapper
#ifdef __cplusplus

inline std::string IPv6ToStrFull(const ipv6_t& IP)
{
    /// @see ::ipv6_to_str_full
    std::string res;
    res.resize( ipv6_to_str_full(NULL, 0, IP) - 1 );
    ipv6_to_str_full((char*)res.c_str(), res.length()+1, IP);
    return res;
}

inline std::string IPv6ToStrShort(const ipv6_t& IP)
{
    /// @see ::ipv6_to_str_short
    std::string res;
    res.resize( ipv6_to_str_short(NULL, 0, IP) - 1 );
    ipv6_to_str_short((char*)res.c_str(), res.length()+1, IP);
    return res;
}

inline std::string IPv6ToStr(const ipv6_t& IP)
{
    /// @see ::ipv6_to_str
    return IPv6ToStrShort(IP);
}

#endif

/**
 * @}
 */

/**
 * @name MAC Address
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/// MAC address
#pragma pack(push,8)
typedef struct macaddr_t
{
    uint8_t dig[6];  ///< 6 digits.
} macaddr_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(macaddr_t) == 6 ); ///< @private

INLINE bool macaddr_is_equal(macaddr_t addr1, macaddr_t addr2)
{
    /**
     * 比對兩個 MAC address 是否相等。
     *
     * @param addr1, addr2 欲比較的兩個 MAC address。
     * @return 傳回兩 MAC address 是否相等的邏輯。
     */
    return ( addr1.dig[0] == addr2.dig[0] )&&
           ( addr1.dig[1] == addr2.dig[1] )&&
           ( addr1.dig[2] == addr2.dig[2] )&&
           ( addr1.dig[3] == addr2.dig[3] )&&
           ( addr1.dig[4] == addr2.dig[4] )&&
           ( addr1.dig[5] == addr2.dig[5] );
}

INLINE macaddr_t macaddr_from_digits(int d0, int d1, int d2, int d3, int d4, int d5)
{
    /**
     * 指定個別的數字來建立一個 IP。
     *
     * @param d0, d1, d2, d3, d4, d5 由頭到尾的 6 個數字，值域為 0 至 255。
     * @return 由傳入參數所組合建立的 MAC address。
     */
    macaddr_t addr;
    addr.dig[0] = d0;
    addr.dig[1] = d1;
    addr.dig[2] = d2;
    addr.dig[3] = d3;
    addr.dig[4] = d4;
    addr.dig[5] = d5;
    return addr;
}

size_t macaddr_to_str(char* dest, size_t destsz, macaddr_t addr);

macaddr_t macaddr_from_str(const char* str);

#ifdef __cplusplus
}  // extern "C"
#endif

// C++ Wrapper
#ifdef __cplusplus

inline std::string MacAddrToStr(const macaddr_t& Addr)
{
    /// @see ::macaddr_to_str
    std::string res;
    res.resize( macaddr_to_str(NULL, 0, Addr) - 1 );
    macaddr_to_str((char*)res.c_str(), res.length()+1, Addr);
    return res;
}

#endif

/**
 * @}
 */

#endif

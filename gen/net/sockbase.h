/**
 * @file
 * @brief     Socket base.
 * @details   Basic definition and tool about sockets.
 * @author    王文佑
 * @date      2014.03.12
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_SOCKBASE_H_
#define _GEN_NET_SOCKBASE_H_

#include "../type.h"
#include "ipconst.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------
//---- Network Tools -------------------
//--------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

ipv4_t socknet_get_ip_by_hostname(const char *hostname);

#ifdef __cplusplus
}  // extern "C"
#endif

//--------------------------------------
//---- Socket Address ------------------
//--------------------------------------

/**
 * @class sockaddr_t
 * @brief Encapsulation of struct sockaddr.
 */
#pragma pack(push,1)
typedef struct sockaddr_t
{
    uint16_t family;
    uint16_t port;
    ipv4_t   addr;
    byte_t   zeros[8];
} sockaddr_t;
#pragma pack(pop)
STATIC_ASSERT( sizeof(sockaddr_t) == 16 );

void     sockaddr_init        (      sockaddr_t *addr);
void     sockaddr_init_value  (      sockaddr_t *addr, ipv4_t ip, uint16_t port);
bool     sockaddr_is_available(const sockaddr_t *addr);
uint16_t sockaddr_get_port    (const sockaddr_t *addr);
ipv4_t   sockaddr_get_ip      (const sockaddr_t *addr);
void     sockaddr_set_port    (      sockaddr_t *addr, uint16_t port);
void     sockaddr_set_ip      (      sockaddr_t *addr, ipv4_t ip);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of sockaddr_t
class TSocketAddr : protected sockaddr_t
{
    friend class TSocketTCP;
    friend class TSocketUDP;

public:
    TSocketAddr()                                {        sockaddr_init        (this); }            ///< @see sockaddr_t::sockaddr_init
    TSocketAddr(const ipv4_t &IP, uint16_t Port) {        sockaddr_init_value  (this, IP, Port); }  ///< @see sockaddr_t::sockaddr_init_value
    TSocketAddr(const sockaddr_t &Src)           { *(sockaddr_t*)this = Src; }                      ///< Copy constructor.

public:
    bool     IsAvailable() const                 { return sockaddr_is_available(this); }        ///< @see sockaddr_t::sockaddr_is_available
    uint16_t GetPort    () const                 { return sockaddr_get_port    (this); }        ///< @see sockaddr_t::sockaddr_get_port
    ipv4_t   GetIP      () const                 { return sockaddr_get_ip      (this); }        ///< @see sockaddr_t::sockaddr_get_ip
    void     SetPort    (uint16_t Port)          {        sockaddr_set_port    (this, Port); }  ///< @see sockaddr_t::sockaddr_set_port
    void     SetIP      (const ipv4_t &IP)       {        sockaddr_set_ip      (this, IP); }    ///< @see sockaddr_t::sockaddr_set_ip

};

STATIC_ASSERT( sizeof(TSocketAddr) == sizeof(sockaddr_t) );

/// C++ wrapper of IP constant
namespace Socket
{

static const ipv4_t   AnyIP       = ipv4_const_any;        ///< Unspecific IP.
static const ipv4_t   LoopIP      = ipv4_const_loop;       ///< Loop back IP.
static const ipv4_t   BroadcastIP = ipv4_const_broadcast;  ///< Broadcast IP.
static const uint16_t AnyPort     = 0;                     ///< Unspecific port.

}  // namespace Socket

#endif  // __cplusplus

//--------------------------------------
//---- Socket Base ---------------------
//--------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Socket file descriptor, or handler.
#ifdef _WIN32
    typedef uintptr_t sockfd_t;
#else
    typedef int       sockfd_t;
#endif

bool       sockfd_is_valid          (sockfd_t sockfd);
sockfd_t   sockfd_get_invalid       (void);
sockaddr_t sockfd_get_local_addr    (sockfd_t sockfd);
sockaddr_t sockfd_get_remote_addr   (sockfd_t sockfd);
void       sockfd_set_block_flag    (sockfd_t sockfd, bool block);
bool       sockfd_set_broadcast_flag(sockfd_t sockfd, bool enable);
int        sockfd_send              (sockfd_t sockfd, const void* data, size_t size);
int        sockfd_receive           (sockfd_t sockfd, void *buf, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif

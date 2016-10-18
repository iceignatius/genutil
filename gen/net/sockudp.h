/**
 * @file
 * @brief     Socket UDP.
 * @details   Encapsulation of UDP type sockets.
 * @author    王文佑
 * @date      2014.11.17
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_SOCKUDP_H_
#define _GEN_NET_SOCKUDP_H_

#include "sockbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @class sockudp_t
 * @brief UDP type socket.
 *
 * @attention
 *     @li The object need to be initialized by calling "[prefix]_init[suffix]" before using,
 *         and call "[prefix]_deinit" when it will not be used any more.
 *     @li The object can be create and initialize dynamically by calling "[prefix]_create[suffix]";
 *         and it will need to call "[prefix]_release" instead of "[prefix]_deinit"
 *         when it will not be used any more.
 *     @li Never call "[prefix]_init[suffix]" to initialize an object which be created by
 *         "[prefix]_create[suffix], because the object was already initialized.
 */
typedef struct sockudp_t
{
    // WARNING : All variables are private!
    sockfd_t   socket;
    sockaddr_t remote;
} sockudp_t;

void       sockudp_init              (      sockudp_t *self);
void       sockudp_deinit            (      sockudp_t *self);

sockfd_t   sockudp_get_fd            (      sockudp_t *self);

void       sockudp_set_block_flag    (      sockudp_t *self, bool block);
void       sockudp_set_block_mode    (      sockudp_t *self);
void       sockudp_set_nonblock_mode (      sockudp_t *self);

bool       sockudp_set_broadcast_flag(      sockudp_t *self, bool enable);
bool       sockudp_enable_broadcast  (      sockudp_t *self);
bool       sockudp_disable_broadcast (      sockudp_t *self);

sockaddr_t sockudp_get_local_addr    (const sockudp_t *self);
sockaddr_t sockudp_get_remote_addr   (const sockudp_t *self);
void       sockudp_set_remote_addr   (      sockudp_t *self, const sockaddr_t *addr);

bool       sockudp_open              (      sockudp_t *self, const sockaddr_t *localaddr);
void       sockudp_close             (      sockudp_t *self);
bool       sockudp_is_opened         (const sockudp_t *self);

int        sockudp_send_to           (      sockudp_t *self, const void* data, size_t size, const sockaddr_t *addr);
int        sockudp_receive_from      (      sockudp_t *self, void* buf, size_t size, sockaddr_t *addr);
int        sockudp_send              (      sockudp_t *self, const void *data, size_t size);
int        sockudp_receive           (      sockudp_t *self, void *buf, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of sockudp_t
class TSocketUDP : protected sockudp_t
{
public:
    TSocketUDP()                                                               {        sockudp_init              (this); }
    ~TSocketUDP()                                                              {        sockudp_deinit            (this); }
private:
    TSocketUDP(const TSocketUDP &Src);             // Not allowed to use
    TSocketUDP& operator=(const TSocketUDP &Src);  // Not allowed to use

public:
    sockfd_t    GetFd()                                                        { return sockudp_get_fd            (this); }                       ///< @see sockudp_t::sockudp_get_fd

    void        SetBlockFlag(bool Block)                                       {        sockudp_set_block_flag    (this, Block); }                ///< @see sockudp_t::sockudp_set_block_flag
    void        SetBlockMode()                                                 {        sockudp_set_block_mode    (this); }                       ///< @see sockudp_t::sockudp_set_block_mode
    void        SetNonblockMode()                                              {        sockudp_set_nonblock_mode (this); }                       ///< @see sockudp_t::sockudp_set_nonblock_mode

    bool        SetBroadcastFlag(bool Enable)                                  { return sockudp_set_broadcast_flag(this, Enable); }               ///< @see sockudp_t::sockudp_set_broadcast_flag
    bool        EnableBroadcast()                                              { return sockudp_enable_broadcast  (this); }                       ///< @see sockudp_t::sockudp_enable_broadcast
    bool        DisableBroadcast()                                             { return sockudp_disable_broadcast (this); }                       ///< @see sockudp_t::sockudp_disable_broadcast

    TSocketAddr GetLocalAddr()                                           const { return sockudp_get_local_addr    (this); }                       ///< @see sockudp_t::sockudp_get_local_addr
    TSocketAddr GetRemoteAddr()                                          const { return sockudp_get_remote_addr   (this); }                       ///< @see sockudp_t::sockudp_get_remote_addr
    void        SetRemoteAddr(const TSocketAddr &Addr)                         {        sockudp_set_remote_addr   (this, &Addr); }                ///< @see sockudp_t::sockudp_set_remote_addr

    bool        Open(const TSocketAddr &LocalAddr)                             { return sockudp_open              (this, &LocalAddr); }           ///< @see sockudp_t::sockudp_open
    void        Close()                                                        {        sockudp_close             (this); }                       ///< @see sockudp_t::sockudp_close

    int         Send(const void *Data, size_t Size)                            { return sockudp_send              (this, Data, Size); }           ///< @see sockudp_t::sockudp_send
    int         Receive(void *Buffer, size_t Size)                             { return sockudp_receive           (this, Buffer, Size); }         ///< @see sockudp_t::sockudp_receive
    int         SendTo(const void *Data, size_t Size, const TSocketAddr &Addr) { return sockudp_send_to           (this, Data, Size, &Addr); }    ///< @see sockudp_t::sockudp_send_to
    int         ReceiveFrom(void *Buffer, size_t Size, TSocketAddr &Addr)      { return sockudp_receive_from      (this, Buffer, Size, &Addr); }  ///< @see sockudp_t::sockudp_receive_from

};

#endif  // __cplusplus

#endif

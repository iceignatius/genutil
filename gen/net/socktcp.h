/**
 * @file
 * @brief     Socket TCP.
 * @details   Encapsulation of TCP type sockets.
 * @author    王文佑
 * @date      2014.11.17
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_SOCKTCP_H_
#define _GEN_NET_SOCKTCP_H_

#include "sockbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @class socktcp_t
 * @brief TCP type socket.
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
typedef struct socktcp_t
{
    // WARNING : All variables are private!
    sockfd_t socket;
} socktcp_t;

void       socktcp_init             (      socktcp_t *self);
void       socktcp_deinit           (      socktcp_t *self);
socktcp_t* socktcp_create           (sockfd_t socket);
void       socktcp_release          (      socktcp_t *self);

sockfd_t   socktcp_get_fd           (      socktcp_t *self);

void       socktcp_set_block_flag   (      socktcp_t *self, bool block_mode);
void       socktcp_set_block_mode   (      socktcp_t *self);
void       socktcp_set_nonblock_mode(      socktcp_t *self);

sockaddr_t socktcp_get_local_addr   (const socktcp_t *self);
sockaddr_t socktcp_get_remote_addr  (const socktcp_t *self);

bool       socktcp_connect          (      socktcp_t *self, const sockaddr_t *addr, unsigned timeout);
bool       socktcp_listen           (      socktcp_t *self, const sockaddr_t *localaddr, bool reuse);
void       socktcp_close            (      socktcp_t *self);
bool       socktcp_is_opened        (const socktcp_t *self);

socktcp_t* socktcp_get_new_connect  (      socktcp_t *self);

int        socktcp_send             (      socktcp_t *self, const void *data, size_t size);
int        socktcp_receive          (      socktcp_t *self, void *buf, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of socktcp_t
class TSocketTCP : protected socktcp_t
{
public:
    TSocketTCP()                                                      {        socktcp_init             (this); }
    ~TSocketTCP()                                                     {        socktcp_deinit           (this); }
private:
    TSocketTCP(const TSocketTCP &Src);             // Not allowed to use
    TSocketTCP& operator=(const TSocketTCP &Src);  // Not allowed to use

public:
    void        Release()                                             {        socktcp_release          (this); }                       ///< @see socktcp_t::socktcp_release

public:
    sockfd_t    GetFd()                                               { return socktcp_get_fd           (this); }                       ///< @see socktcp_t::socktcp_get_fd

    void        SetBlockFlag(bool BlockMode)                          {        socktcp_set_block_flag   (this, BlockMode); }            ///< @see socktcp_t::socktcp_set_block_flag
    void        SetBlockMode()                                        {        socktcp_set_block_mode   (this); }                       ///< @see socktcp_t::socktcp_set_block_mode
    void        SetNonblockMode()                                     {        socktcp_set_nonblock_mode(this); }                       ///< @see socktcp_t::socktcp_set_nonblock_mode

    TSocketAddr GetLocalAddr()                                  const { return socktcp_get_local_addr   (this); }                       ///< @see socktcp_t::socktcp_get_local_addr
    TSocketAddr GetRemoteAddr()                                 const { return socktcp_get_remote_addr  (this); }                       ///< @see socktcp_t::socktcp_get_remote_addr

    bool        Connect(const TSocketAddr &Addr, unsigned Timeout=-1) { return socktcp_connect          (this, &Addr, Timeout); }       ///< @see socktcp_t::socktcp_connect
    bool        Listen(const TSocketAddr &LocalAddr, bool Reuse)      { return socktcp_listen           (this, &LocalAddr, Reuse); }    ///< @see socktcp_t::socktcp_listen
    void        Close()                                               {        socktcp_close            (this); }                       ///< @see socktcp_t::socktcp_close

    TSocketTCP* GetNewConnect()                                       { return (TSocketTCP*) socktcp_get_new_connect(this); }           ///< @see socktcp_t::socktcp_get_new_connect

    int         Send(const void *Data, size_t Size)                   { return socktcp_send             (this, Data, Size); }           ///< @see socktcp_t::socktcp_send
    int         Receive(void *Buffer, size_t Size)                    { return socktcp_receive          (this, Buffer, Size); }         ///< @see socktcp_t::socktcp_receive

};

#endif  // __cplusplus

#endif

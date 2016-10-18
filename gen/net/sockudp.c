#include <assert.h>

#ifdef __linux__
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "winwsa.h"
#include "sockudp.h"

//------------------------------------------------------------------------------
void sockudp_init(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Constructor.
     *
     * @param self Object instance.
     */
    assert( self );

    winwsa_init_recursive();
    self->socket = sockfd_get_invalid();
    sockaddr_init(&self->remote);
}
//------------------------------------------------------------------------------
void sockudp_deinit(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Destructor.
     *
     * @param self Object instance.
     */
    assert( self );

    sockudp_close(self);
    winwsa_deinit_recursive();
}
//------------------------------------------------------------------------------
sockfd_t sockudp_get_fd(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Get the OS depend socket file descriptor.
     *
     * @param self Object instance.
     * @return The socket file descriptor.
     */
    assert( self );

    return self->socket;
}
//------------------------------------------------------------------------------
void sockudp_set_block_flag(sockudp_t *self, bool block)
{
    /**
     * @memberof sockudp_t
     * @brief Set blocking mode by flag.
     *
     * @param self  Object instance.
     * @param block TRUE to set blocking mode; and FALSE to set non-blocking mode.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, block);
}
//------------------------------------------------------------------------------
void sockudp_set_block_mode(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Set to blocking mode.
     *
     * @param self Object instance.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, true);
}
//------------------------------------------------------------------------------
void sockudp_set_nonblock_mode(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Set to non-blocking mode.
     *
     * @param self Object instance.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, false);
}
//------------------------------------------------------------------------------
bool sockudp_set_broadcast_flag(sockudp_t *self, bool enable)
{
    /**
     * @memberof sockudp_t
     * @brief Enable or disable broadcast by flag.
     *
     * @param self   Object instance.
     * @param enable TRUE to enable broadcast; and FALSE to disable it.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( self );

    return sockfd_set_broadcast_flag(self->socket, enable);
}
//------------------------------------------------------------------------------
bool sockudp_enable_broadcast(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Enable broadcast.
     *
     * @param self Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( self );

    return sockfd_set_broadcast_flag(self->socket, true);
}
//------------------------------------------------------------------------------
bool sockudp_disable_broadcast(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Disable broadcast.
     *
     * @param self Object instance.
     * @return TRUE if succeed; and FALSE if failed.
     */
    assert( self );

    return sockfd_set_broadcast_flag(self->socket, false);
}
//------------------------------------------------------------------------------
sockaddr_t sockudp_get_local_addr(const sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Get address information of the local host.
     *
     * @param self Object instance.
     * @return Address information of the local host.
     *
     * @remarks This function will be succeed only if the socket have a connection.
     */
    assert( self );

    return sockfd_get_local_addr(self->socket);
}
//------------------------------------------------------------------------------
sockaddr_t sockudp_get_remote_addr(const sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Get address information of the remote host.
     *
     * @param self Object instance.
     * @return Address information of the remote host.
     */
    assert( self );

    return self->remote;
}
//------------------------------------------------------------------------------
void sockudp_set_remote_addr(sockudp_t *self, const sockaddr_t *addr)
{
    /**
     * @memberof sockudp_t
     * @brief Set address information of the remote host.
     *
     * @param self Object instance.
     * @param addr Address information of the remote host.
     */
    assert( self && addr );

    self->remote = *addr;
}
//------------------------------------------------------------------------------
bool sockudp_open(sockudp_t *self, const sockaddr_t *localaddr)
{
    /**
     * @memberof sockudp_t
     * @brief Activate the socket object for network working.
     *
     * @param self      Object instance.
     * @param localaddr Set the local listening address :
     *                  @li The IP can be ZERO to let system select a valid IP to use.
     *                      But note that we cannot get the IP the system selected later.
     *                  @li The port can be set to a valid number manually,
     *                      or just set to ZERO to let system select a valid port number.
     *                      And we can get the system selected port number later.
     * @return TRUE if succeed; and FALSE if not.
     *
     * @remarks The new connection will be in non-blocking mode,
     *          and need be change to blocking mode if you want to using it under blocking mode.
     */
    bool succeed = false;

    assert( self );

    do
    {
        // Terminate current work
        sockudp_close(self);

        // WSA startup
        if( !winwsa_start_explicit() ) break;

        // Create socket
        self->socket = socket(AF_INET, SOCK_DGRAM, 0);
        if( !sockfd_is_valid(self->socket) ) break;

        // Bind socket
        if( bind(self->socket, (const struct sockaddr*)localaddr, sizeof(sockaddr_t)) ) break;

        // Set socket to non-blocking mode
        sockfd_set_block_flag(self->socket, false);

        succeed = true;
    } while( false );

    // Release socket if open failed
    if( !succeed ) sockudp_close(self);

    return succeed;
}
//------------------------------------------------------------------------------
void sockudp_close(sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Deactivate the network working.
     *
     * @param self Object instance.
     */
    assert( self );

    if( sockfd_is_valid(self->socket) )
    {
#if   defined(__linux__)
        close(self->socket);
#elif defined(_WIN32)
        closesocket(self->socket);
#else
    #error No implementation on this platform!
#endif
        self->socket = sockfd_get_invalid();
    }
}
//------------------------------------------------------------------------------
bool sockudp_is_opened(const sockudp_t *self)
{
    /**
     * @memberof sockudp_t
     * @brief Check if the socket is opened.
     *
     * @param self Object instance.
     * @return TRUE if the socket is opened; and FALSE if not.
     */
    assert( self );
    return sockfd_is_valid(self->socket);
}
//------------------------------------------------------------------------------
int sockudp_send_to(sockudp_t *self, const void* data, size_t size, const sockaddr_t *addr)
{
    /**
     * @memberof sockudp_t
     * @brief Send data to a specified remote host.
     *
     * @param self Object instance.
     * @param data Data to send.
     * @param size Size of data to send.
     * @param addr The target address to send data to.
     * @return The size sent if succeed; and -1 if failed.
     */
    int sentsz;
#ifdef _WIN32
    int flags = 0;
#else
    int flags = MSG_NOSIGNAL;
#endif

    assert( self && addr );
    if( !data || !size ) return 0;

    sentsz = sendto(self->socket, data, size, flags, (const struct sockaddr*)addr, sizeof(sockaddr_t));
    return ( sentsz >= 0 )?( sentsz ):( -1 );
}
//------------------------------------------------------------------------------
int sockudp_receive_from(sockudp_t *self, void* buf, size_t size, sockaddr_t *addr)
{
    /**
     * @memberof sockudp_t
     * @brief Receive data from a specified target.
     *
     * @param self Object instance.
     * @param buf  A buffer to receive data.
     * @param size Size of the buffer.
     * @param addr To receive the address information about where the data come from if we have data come in.
     * @return The size received if succeed; and -1 if failed.
     */
#ifdef __linux__
    socklen_t addrlen = sizeof(sockaddr_t);
#else
    int       addrlen = sizeof(sockaddr_t);
#endif
    int       recsz;

    assert( self && addr );
    if( !buf || !size ) return 0;

    sockaddr_init(addr);

    recsz = recvfrom(self->socket, buf, size, 0, (struct sockaddr*)addr, &addrlen);
    if( recsz < 0 )
    {
#if   defined(__linux__)
    recsz = ( errno == EAGAIN )?( 0 ):( -1 );
#elif defined(_WIN32)
    recsz = ( winwsa_get_last_error_code() == WSAEWOULDBLOCK )?( 0 ):( -1 );
#else
    #error No implementation on this platform!
#endif
    }

    return recsz;
}
//------------------------------------------------------------------------------
int sockudp_send(sockudp_t *self, const void *data, size_t size)
{
    /**
     * @memberof sockudp_t
     * @brief Send data to the remote host.
     *
     * @param self Object instance.
     * @param data Data to send.
     * @param size Size of data to send.
     * @return The size sent if succeed; and -1 if failed.
     *
     * @remarks Data will be sent to the remote host by the address setted by sockudp_set_remote_addr.
     */
    assert( self );

    return sockudp_send_to(self, data, size, &self->remote);
}
//------------------------------------------------------------------------------
int sockudp_receive(sockudp_t *self, void *buf, size_t size)
{
    /**
     * @memberof sockudp_t
     * @brief Receive data from anywhere.
     *
     * @param self Object instance.
     * @param buf  A buffer to receive data.
     * @param size Size of the buffer.
     * @return The size received if succeed; and -1 if failed.
     */
    sockaddr_t addr;

    assert( self );

    return sockudp_receive_from(self, buf, size, &addr);
}
//------------------------------------------------------------------------------

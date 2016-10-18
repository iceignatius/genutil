#include <assert.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "winwsa.h"
#include "socktcp.h"

//------------------------------------------------------------------------------
void socktcp_init(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Constructor.
     *
     * @param self Object instance.
     */
    assert( self );

    winwsa_init_recursive();
    self->socket = sockfd_get_invalid();
}
//------------------------------------------------------------------------------
void socktcp_deinit(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Destructor.
     *
     * @param self Object instance.
     */
    assert( self );

    socktcp_close(self);
    winwsa_deinit_recursive();
}
//------------------------------------------------------------------------------
socktcp_t* socktcp_create(sockfd_t socket)
{
    /**
     * @memberof socktcp_t
     * @brief Dynamic create a socktcp_t object.
     *
     * @param socket The system socket file descriptor to be used by this object,
     *               and that socket file descriptor will be close when object be released.
     * @return Return the socket object create if succeed; and NULL if failed.
     */
    socktcp_t *self    = NULL;
    bool       succeed = false;

    do
    {
        self = malloc(sizeof(socktcp_t));
        if( !self ) break;

        socktcp_init(self);
        self->socket = socket;

        succeed = true;
    } while(false);

    if( !succeed )
    {
        if( self )
        {
            free(self);
            self = NULL;
        }
    }

    return self;
}
//------------------------------------------------------------------------------
void socktcp_release(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Release a dynamic created socktcp_t object.
     *
     * @param self Object instance.
     *
     * @warning Call this function on a dynamic created object only!
     */
    assert( self );

    socktcp_deinit(self);
    free(self);
}
//------------------------------------------------------------------------------
sockfd_t socktcp_get_fd(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Get the OS depend socket file descriptor.
     *
     * @param self Object instance.
     * @return The socket file descriptor.
     */
    assert( self );

    return self->socket;
}
//------------------------------------------------------------------------------
void socktcp_set_block_flag(socktcp_t *self, bool block_mode)
{
    /**
     * @memberof socktcp_t
     * @brief Set blocking mode by flag.
     *
     * @param self       Object instance.
     * @param block_mode TRUE to set blocking mode; and FALSE to set non-blocking mode.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, block_mode);
}
//------------------------------------------------------------------------------
void socktcp_set_block_mode(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Set to blocking mode.
     *
     * @param self Object instance.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, true);
}
//------------------------------------------------------------------------------
void socktcp_set_nonblock_mode(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Set to non-blocking mode.
     *
     * @param self Object instance.
     */
    assert( self );

    sockfd_set_block_flag(self->socket, false);
}
//------------------------------------------------------------------------------
sockaddr_t socktcp_get_local_addr(const socktcp_t *self)
{
    /**
     * @memberof socktcp_t
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
sockaddr_t socktcp_get_remote_addr(const socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Get address information of the remote host.
     *
     * @param self Object instance.
     * @return Address information of the remote host.
     *
     * @remarks This function will be succeed only if the object have a connection.
     */
    assert( self );

    return sockfd_get_remote_addr(self->socket);
}
//------------------------------------------------------------------------------
static
bool connect_with_timeout(sockfd_t fd, const sockaddr_t *addr, unsigned timeout)
{
    if( !connect(fd, (const struct sockaddr*)addr, sizeof(sockaddr_t)) )
        return true;

#if   defined(__linux__)
    if( EINPROGRESS != errno ) return false;
#elif defined(_WIN32)
    if( WSAEWOULDBLOCK != winwsa_get_last_error_code() ) return false;
#else
    #error No implementation on this platform!
#endif

    fd_set wlst;
    FD_ZERO(&wlst);
    FD_SET(fd, &wlst);

    fd_set elst;
    FD_ZERO(&elst);
    FD_SET(fd, &elst);

    struct timeval tv;
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = timeout % 1000 * 1000;

    if( 0 >= select(fd+1, NULL, &wlst, &elst, &tv) ) return false;

#if   defined(__linux__)
    int       optval;
    socklen_t optlen = sizeof(optval);
    if( getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) ) return false;
#elif defined(_WIN32)
    int optval;
    int optlen = sizeof(optval);
    if( getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) ) return false;
#else
    #error No implementation on this platform!
#endif

    return !optval;
}
//------------------------------------------------------------------------------
bool socktcp_connect(socktcp_t *self, const sockaddr_t *addr, unsigned timeout)
{
    /**
     * @memberof socktcp_t
     * @brief Connect to a specified remote host.
     *
     * @param self    Object instance.
     * @param addr    The remote address.
     * @param timeout Time out value in milliseconds.
     *                And the system default value will be used if
     *                the input value be large than that.
     * @return TRUE if succeed; and FALSE if not.
     *
     * @remarks
     *     @li The new connection will be in non-blocking mode,
     *         and need be change to blocking mode if you want to using it under blocking mode.
     *     @li This function will terminate the listen process if the object is listening.
     */
    bool succeed = false;

    assert( self );

    do
    {
        // Terminate existing connection
        socktcp_close(self);

        // WSA Startup
        if( !winwsa_start_explicit() )
            break;

        // Create socket
        self->socket = socket(AF_INET, SOCK_STREAM, 0);
        if( !sockfd_is_valid(self->socket) )
            break;

        // Set socket to non-blocking mode
        sockfd_set_block_flag(self->socket, false);

        // Connect remote host
        if( !connect_with_timeout(self->socket, addr, timeout) ) break;

        succeed = true;
    } while( false );

    // Close socket if connect failed
    if( !succeed ) socktcp_close(self);

    return succeed;
}
//------------------------------------------------------------------------------
bool socktcp_listen(socktcp_t *self, const sockaddr_t *localaddr)
{
    /**
     * @memberof socktcp_t
     * @brief Starting to listen the client connect request.
     *
     * @param self      Object instance.
     * @param localaddr Set the local listening address :
     *                  @li The IP can be ZERO (::ipv4_const_any) to let system select a valid IP to use.
     *                      But note that we cannot get the IP that system selected later.
     *                  @li The port can be set to a valid number manually,
     *                      or just set to ZERO to let system select a valid port number.
     *                      And we can get the system selected port number later.
     * @return TRUE if succeed; and FALSE if not.
     *
     * @remarks
     *     @li The new connection will be in non-blocking mode,
     *         and need be change to blocking mode if you want to using it under blocking mode.
     *     @li This function will terminate the current connection if the object is connecting to a host.
     *     @li You can use socktcp_get_new_connect to check and get the new connections
     *         after the object go in listening process succeed.
     */
    bool succeed = false;

    assert( self );

    do
    {
        // Terminate existing connection
        socktcp_close(self);

        // WSA Startup
        if( !winwsa_start_explicit() ) break;

        // Create socket
        self->socket = socket(AF_INET, SOCK_STREAM, 0);
        if( !sockfd_is_valid(self->socket) ) break;

        // Bind socket
        if( bind(self->socket, (const struct sockaddr*)localaddr, sizeof(sockaddr_t)) ) break;

        // Stert listen
        if( listen(self->socket, SOMAXCONN) ) break;

        // Set socket to non-blocking mode
        sockfd_set_block_flag(self->socket, false);

        succeed = true;
    } while( false );

    // Release socket if listen start failed
    if( !succeed ) socktcp_close(self);

    return succeed;
}
//------------------------------------------------------------------------------
void socktcp_close(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Terminate the current connection or listen process.
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
bool socktcp_is_opened(const socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Check if the socket is opened.
     *
     * @param self The object to be operated.
     * @return TRUE if it is opened; and FALSE if not.
     *
     * @remarks  This function only indicate if it have an available socket instance to use,
     *           The connection may be broken even if this function returns TRUE.
     *           Actually, the socket cannot be sure to got the disconnect message at any cases.
     *           User should use other methods (echo, polling, ... etc.)
     *           to check if the connection still available.
     */
    assert( self );
    return sockfd_is_valid(self->socket);
}
//------------------------------------------------------------------------------
socktcp_t* socktcp_get_new_connect(socktcp_t *self)
{
    /**
     * @memberof socktcp_t
     * @brief Check and get the new client connection.
     *
     * @param self Object instance.
     * @return A TCP socket object will be returned if there have a new connection be established;
     *         and NULL will be returned otherwise.
     *
     * @remarks
     *     @li This function will net get any connection if the object is not in listening process,
     *         you will need to call socktcp_listen to start listen process.
     *     @li You can make your communication with the client by using the object returned by this function,
     *         but remember that, you MUST call socktcp_release to release the object when the time
     *         you not need the connection any more.
     *     @li The new connection will be in non-blocking mode,
     *         and need be change to blocking mode if you want to using it under blocking mode.
     */
    sockfd_t    peerfd;
    sockaddr_t  peeraddr;
    socktcp_t  *peer;
#ifdef __linux__
    socklen_t   addrsz = sizeof(sockaddr_t);
#else
    int         addrsz = sizeof(sockaddr_t);
#endif

    assert( self );

    // Check new connect
    peerfd = accept(self->socket, (struct sockaddr*)&peeraddr, &addrsz);
    if( !sockfd_is_valid(peerfd) ) return NULL;

    // Create peer object
    peer = socktcp_create(peerfd);
    if( !peer ) return NULL;

    // Set peer to non-block mode
    socktcp_set_nonblock_mode(peer);

    return peer;
}
//------------------------------------------------------------------------------
int socktcp_send(socktcp_t *self, const void *data, size_t size)
{
    /**
     * @memberof socktcp_t
     * @brief Send data to the remote host.
     *
     * @param self Object instance.
     * @param data Data to send.
     * @param size Size of data to send.
     * @return The size sent if succeed; and -1 if failed.
     */
    assert( self );

    return sockfd_send(self->socket, data, size);
}
//------------------------------------------------------------------------------
int socktcp_receive(socktcp_t *self, void *buf, size_t size)
{
    /**
     * @memberof socktcp_t
     * @brief Receive data from the remote host.
     *
     * @param self Object instance.
     * @param buf  A buffer to receive data.
     * @param size Size of the buffer.
     * @return The size received if succeed; and -1 if failed.
     */
    assert( self );

    return sockfd_receive(self->socket, buf, size);
}
//------------------------------------------------------------------------------

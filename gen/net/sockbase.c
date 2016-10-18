#include <assert.h>
#include <string.h>

#ifdef __linux__
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../endian.h"
#include "winwsa.h"
#include "sockbase.h"

//------------------------------------------------------------------------------
//---- Network Tools -----------------------------------------------------------
//------------------------------------------------------------------------------
ipv4_t socknet_get_ip_by_hostname(const char *hostname)
{
    /**
     * Get host IP by name.
     *
     * @param hostname The host name to search, for example : "www.google.com".
     * @return The IP of the host if search successfully; or ipv4_const_invalid if failed.
     *
     * @remarks If the host have multiple IPs, the first will be returned only.
     */
    ipv4_t ip = ipv4_const_invalid;

    winwsa_init_recursive();

    do
    {
        struct hostent *host = gethostbyname(hostname);
        if( !host ) break;

        if( host->h_length > 4 ) break;     // This function support for IPv4 only.

        if( !host->h_addr_list[0] ) break;  // Failure because no any address here.

        memcpy(&ip, host->h_addr_list[0], host->h_length);

    } while(false);

    winwsa_deinit_recursive();

    return ip;
}
//------------------------------------------------------------------------------
//---- Socket Address ----------------------------------------------------------
//------------------------------------------------------------------------------
void sockaddr_init(sockaddr_t *addr)
{
    /**
     * @memberof sockaddr_t
     * @brief Constructor.
     *
     * @param addr Object instance.
     */
    assert( addr );

    memset(addr, 0, sizeof(*addr));
    addr->family = AF_INET;
}
//------------------------------------------------------------------------------
void sockaddr_init_value(sockaddr_t *addr, ipv4_t ip, uint16_t port)
{
    /**
     * @memberof sockaddr_t
     * @brief   Constructor.
     * @details Construct with initial values.
     *
     * @param addr Object instance.
     * @param ip   IP of the address.
     * @param port Port of the address.
     */
    assert( addr );

    memset(addr, 0, sizeof(*addr));
    addr->family = AF_INET;
    addr->addr   = ip;
    addr->port   = endian_local_to_be_16(port);
}
//------------------------------------------------------------------------------
bool sockaddr_is_available(const sockaddr_t *addr)
{
    /**
     * @memberof sockaddr_t
     * @brief 傳回本物件是否具有有效的資料。
     *
     * @param addr Object instance.
     * @return TRUE if the address is available; and FALSE if not.
     */
    assert( addr );

    return addr->family && addr->port && addr->addr.val;
}
//------------------------------------------------------------------------------
uint16_t sockaddr_get_port(const sockaddr_t *addr)
{
    /**
     * @memberof sockaddr_t
     * @brief Get port number of the address.
     *
     * @param addr Object instance.
     * @return The port number.
     */
    assert( addr );

    return endian_be_to_local_16(addr->port);
}
//------------------------------------------------------------------------------
ipv4_t sockaddr_get_ip(const sockaddr_t *addr)
{
    /**
     * @memberof sockaddr_t
     * @brief Get IP of the address.
     *
     * @param addr Object instance.
     * @return The IP.
     */
    assert( addr );

    return addr->addr;
}
//------------------------------------------------------------------------------
void sockaddr_set_port(sockaddr_t *addr, uint16_t port)
{
    /**
     * @memberof sockaddr_t
     * @brief Set port number of the address.
     *
     * @param addr Object instance.
     * @param port The new port number to set.
     */
    assert( addr );

    addr->port = endian_local_to_be_16(port);
}
//------------------------------------------------------------------------------
void sockaddr_set_ip(sockaddr_t *addr, ipv4_t ip)
{
    /**
     * @memberof sockaddr_t
     * @brief Set IP of the address.
     *
     * @param addr Object instance.
     * @param ip   The new IP to set.
     */
    assert( addr );

    addr->addr = ip;
}
//------------------------------------------------------------------------------
//---- Socket Base -------------------------------------------------------------
//------------------------------------------------------------------------------
bool sockfd_is_valid(sockfd_t sockfd)
{
    /*
     * Check if the socket valid.
     */
#ifdef _WIN32
    return sockfd != INVALID_SOCKET;
#else
    return sockfd >= 0;
#endif
}
//------------------------------------------------------------------------------
sockfd_t sockfd_get_invalid(void)
{
    /*
     * Get an invalid socket.
     */
#ifdef _WIN32
    return INVALID_SOCKET;
#else
    return -1;
#endif
}
//------------------------------------------------------------------------------
sockaddr_t sockfd_get_local_addr(sockfd_t sockfd)
{
    /*
     * Get address information of the local host.
     *
     * @param sockfd The socket to be operated.
     * @return Address information of the local host.
     *
     * @remarks This function will be succeed only if the socket have a connection.
     */
    sockaddr_t addr;
#ifdef __linux__
    socklen_t  addrlen = sizeof(addr);
#else
    int        addrlen = sizeof(addr);
#endif

    if( getsockname(sockfd, (struct sockaddr*)&addr, &addrlen) )
        memset(&addr, 0, sizeof(addr));

    return addr;
}
//------------------------------------------------------------------------------
sockaddr_t sockfd_get_remote_addr(sockfd_t sockfd)
{
    /*
     * Get address information of the remote host.
     *
     * @param sockfd The socket to be operated.
     * @return Address information of the remote host.
     *
     * @remarks This function will be succeed only if the socket have a connection.
     */
    sockaddr_t addr;
#ifdef __linux__
    socklen_t  addrlen = sizeof(addr);
#else
    int        addrlen = sizeof(addr);
#endif

    if( getpeername(sockfd, (struct sockaddr*)&addr, &addrlen) )
        memset(&addr, 0, sizeof(addr));

    return addr;
}
//------------------------------------------------------------------------------
void sockfd_set_block_flag(sockfd_t sockfd, bool block)
{
    /*
     * Set socket to blocking or non-blocking mode.
     *
     * @param sockfd The socket to be operated.
     * @param block  TRUE to go in blocking mode; FALSE to non-blocking mode.
     */
#if   defined(__linux__)
    int flags;

    // Get current flags
    flags = fcntl(sockfd, F_GETFL, 0);
    if( flags == -1 ) return;

    // Add or remove block mode to the flags
    if( block ) flags &= ~O_NONBLOCK;
    else        flags |=  O_NONBLOCK;

    // Set flags to socket
    fcntl(sockfd, F_SETFL, flags);
#elif defined(_WIN32)
    u_long flag = ( block )?( FALSE ):( TRUE );
    ioctlsocket(sockfd, FIONBIO, &flag);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool sockfd_set_broadcast_flag(sockfd_t sockfd, bool enable)
{
    /*
     * Enable or disable broadcast.
     *
     * @param sockfd The socket to be operated.
     * @param enable TRUE to enable broadcast; and FALSE to disable it.
     * @return TRUE if succeed; and FALSE if failed.
     */
#ifdef _WIN32
    BOOL flag = true;
#else
    int  flag = true;
#endif

    return !setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&flag, sizeof(flag));
}
//------------------------------------------------------------------------------
int sockfd_send(sockfd_t sockfd, const void* data, size_t size)
{
    /*
     * Send data to the remote host.
     *
     * @param sockfd The socket to be operated.
     * @param data   Data to send.
     * @param size   Size of data to send.
     * @return Return the size sent if succeed; and -1 if failed.
     */
    int sentsz;

#ifdef _WIN32
    int sendflag = 0;
#else
    int sendflag = MSG_NOSIGNAL;
#endif

    if( !data || !size ) return 0;

    sentsz = send(sockfd, data, size, sendflag);
    if( sentsz > 0 )
    {
        // Success.
        return sentsz;
    }
    else if( sentsz < 0 )
    {
        // Error occurred, check if that means "try again" error.
#if   defined(__linux__)
        return ( errno == EAGAIN )?( 0 ):( -1 );
#elif defined(_WIN32)
        return ( winwsa_get_last_error_code() == WSAEWOULDBLOCK )?( 0 ):( -1 );
#else
    #error No implementation on this platform!
#endif
    }
    else
    {
        // Remote disconnected.
#if   defined(__linux__)
        errno = EHOSTDOWN;
#elif defined(_WIN32)
        winwsa_set_last_error_code(WSAEHOSTDOWN);
#else
    #error No implementation on this platform!
#endif
        return -1;
    }
}
//------------------------------------------------------------------------------
int sockfd_receive(sockfd_t sockfd, void *buf, size_t size)
{
    /*
     * Receive data from the remote host.
     *
     * @param sockfd The socket to be operated.
     * @param buf    A buffer to receive data.
     * @param size   Size of the buffer.
     * @return Return the size received if succeed; and -1 if failed.
     */
    int recsz;

    if( !buf || !size ) return 0;

    recsz = recv(sockfd, buf, size, 0);
    if( recsz > 0 )
    {
        // Success.
        return recsz;
    }
    else if( recsz < 0 )
    {
        // Error occurred, check if that means "try again" error.
#if   defined(__linux__)
        return ( errno == EAGAIN )?( 0 ):( -1 );
#elif defined(_WIN32)
        return ( winwsa_get_last_error_code() == WSAEWOULDBLOCK )?( 0 ):( -1 );
#else
    #error No implementation on this platform!
#endif
    }
    else
    {
        // Remote disconnected.
#if   defined(__linux__)
        errno = EHOSTDOWN;
#elif defined(_WIN32)
        winwsa_set_last_error_code(WSAEHOSTDOWN);
#else
    #error No implementation on this platform!
#endif
        return -1;
    }
}
//------------------------------------------------------------------------------

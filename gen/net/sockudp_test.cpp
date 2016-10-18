#include <assert.h>
#include <string.h>
#include <threads.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../systime.h"
#include "sockudp.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

//------------------------------------------------------------------------------
int THRDS_CALL PeerProc1(uint16_t *Port)
{
    TSocketUDP  Peer;
    TSocketAddr Addr;
    int         res;
    char        buf[24];

    // Open port and print information
    res   = Peer.Open(TSocketAddr(Socket::AnyIP, Socket::AnyPort));
    *Port = Peer.GetLocalAddr().GetPort();
    Peer.SetBlockMode();
    printf("[Peer1] : Open : %d\n", res);
    printf("[Peer1] : Local  IP : %s, Port : %u\n",
           IPv4ToStr(Peer.GetLocalAddr().GetIP()).c_str(),
           Peer.GetLocalAddr().GetPort());
    printf("[Peer1] : Remote IP : %s, Port : %u\n",
           IPv4ToStr(Peer.GetRemoteAddr().GetIP()).c_str(),
           Peer.GetRemoteAddr().GetPort());

    // Receive
    memset(buf, 0, sizeof(buf));
    res = Peer.ReceiveFrom(buf, sizeof(buf), Addr);
    systime_sleep(1000);
    printf("[Peer1] : Recv : %d \"%s\"\n", res, buf);
    printf("[Peer1] : Recv IP : %s, Port : %u\n",
           IPv4ToStr(Addr.GetIP()).c_str(),
           Addr.GetPort());

    // Send
    res = Peer.SendTo("Peer1 Response", sizeof("Peer1 Response"), Addr);
    printf("[Peer1] : Send : %d\n", res);

    Peer.Close();

    return 0;
}
//------------------------------------------------------------------------------
int THRDS_CALL PeerProc2(uint16_t* Port)
{
    TSocketUDP  Peer;
    TSocketAddr Addr;
    int         res;
    char        buf[24];

    systime_sleep(1000);

    // Open port and print information
    res = Peer.Open(TSocketAddr(Socket::AnyIP, Socket::AnyPort));
    Peer.SetBlockMode();
    printf("[Peer2] : Open : %d\n", res);
    res = Peer.EnableBroadcast();
    printf("[Peer2] : Enable broadcast : %d\n", res);
    Peer.SetRemoteAddr(TSocketAddr(Socket::BroadcastIP, *Port));
    printf("[Peer2] : Local  IP : %s, Port : %u\n",
           IPv4ToStr(Peer.GetLocalAddr().GetIP()).c_str(),
           Peer.GetLocalAddr().GetPort());
    printf("[Peer2] : Remote IP : %s, Port : %u\n",
           IPv4ToStr(Peer.GetRemoteAddr().GetIP()).c_str(),
           Peer.GetRemoteAddr().GetPort());

    // Empty receive test
    Peer.SetNonblockMode();
    memset(buf, 0, sizeof(buf));
    res = Peer.ReceiveFrom(buf, sizeof(buf), Addr);
    printf("[Peer2] : Recv (Empty Test): %d \"%s\"\n", res, buf);
    printf("[Peer2] : Recv IP : %s, Port : %u\n",
           IPv4ToStr(Addr.GetIP()).c_str(),
           Addr.GetPort());
    Peer.SetBlockMode();

    // Send
    systime_sleep(1000);
    res = Peer.Send("Peer2 Message", sizeof("Peer2 Message"));
    printf("[Peer2] : Send : %d\n", res);

    // Receive
    memset(buf, 0, sizeof(buf));
    res = Peer.ReceiveFrom(buf, sizeof(buf), Addr);
    systime_sleep(1000);
    printf("[Peer2] : Recv : %d \"%s\"\n", res, buf);
    printf("[Peer2] : Recv IP : %s, Port : %u\n",
           IPv4ToStr(Addr.GetIP()).c_str(),
           Addr.GetPort());

    Peer.Close();

    return 0;
}
//------------------------------------------------------------------------------
int main()
{
    /// Global function test

    ipv4_t IP1, IP2;

    IP1 = socknet_get_ip_by_hostname("www.google.com");
    IP2 = socknet_get_ip_by_hostname("tw.yahoo.com");
    if( IP1.val ) printf("IP of \"www.google.com\" : %s\n", IPv4ToStr(IP1).c_str());
    if( IP2.val ) printf("IP of \"tw.yahoo.com\"   : %s\n", IPv4ToStr(IP2).c_str());
    printf("\n");

    /// TCP connection test

    uint16_t ServerPort = Socket::AnyPort;
    thrd_t   Thrd1, Thrd2;
    int      ThrdRes;

    assert( thrd_success == thrd_create(&Thrd1, (thrd_start_t)PeerProc1, &ServerPort) );
    assert( thrd_success == thrd_create(&Thrd2, (thrd_start_t)PeerProc2, &ServerPort) );

    assert( thrd_success == thrd_join(Thrd1, &ThrdRes) );
    assert( thrd_success == thrd_join(Thrd2, &ThrdRes) );

    return 0;
}
//------------------------------------------------------------------------------

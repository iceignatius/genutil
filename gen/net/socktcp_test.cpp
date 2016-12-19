#include <assert.h>
#include <string.h>
#include <threads.h>
#include <stdio.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../systime.h"
#include "socktcp.h"

#ifdef NDEBUG
    #error This test program must work with macro "ASSERT" enabled!
#endif

//------------------------------------------------------------------------------
int THRDS_CALL ServerProc(uint16_t *Port)
{
    TSocketTCP  Server;
    TSocketTCP *Peer;
    int         res;
    char        buf[24];

    // Start lesson and print information
    res   = Server.Listen(TSocketAddr(Socket::AnyIP, Socket::AnyPort), false);
    *Port = Server.GetLocalAddr().GetPort();
    Server.SetBlockMode();
    printf("[Server] : Listen : %d\n", res);
    printf("[Server] : Local  IP : %s, Port : %u\n",
           IPv4ToStr(Server.GetLocalAddr().GetIP()).c_str(),
           Server.GetLocalAddr().GetPort());

    // Get new peer and print information
    Peer = Server.GetNewConnect();
    Peer->SetBlockMode();
    printf("[Server] : New connect : %p\n", Peer);
    printf("[Server] : Stop listen\n");
    Server.Close();
    printf("[Server] : Listen local IP : %s, Port : %u\n",
           IPv4ToStr(Server.GetLocalAddr().GetIP()).c_str(),
           Server.GetLocalAddr().GetPort());
    printf("[Server] : Peer local IP   : %s, Port : %u\n",
           IPv4ToStr(Peer->GetLocalAddr().GetIP()).c_str(),
           Peer->GetLocalAddr().GetPort());
    printf("[Server] : Peer remote IP  : %s, Port : %u\n",
           IPv4ToStr(Peer->GetRemoteAddr().GetIP()).c_str(),
           Peer->GetRemoteAddr().GetPort());

    // Receive
    memset(buf, 0, sizeof(buf));
    res = Peer->Receive(buf, sizeof(buf));
    systime_sleep(1000);
    printf("[Server] : Recv : %d \"%s\"\n", res, buf);

    // Send
    res = Peer->Send("Server Response", sizeof("server Response"));
    printf("[Server] : Send : %d\n", res);

    Peer->Release();

    return 0;
}
//------------------------------------------------------------------------------
int THRDS_CALL ClientProc(uint16_t* Port)
{
    TSocketTCP Client;
    int        res;
    char       buf[24];

    systime_sleep(1000);

    // Connect and print information
    res = Client.Connect(TSocketAddr(Socket::LoopIP, *Port));
    Client.SetBlockMode();
    systime_sleep(1000);
    printf("[Client] : Connect : %d\n", res);
    printf("[Client] : Local  IP : %s, Port : %u\n",
           IPv4ToStr(Client.GetLocalAddr().GetIP()).c_str(),
           Client.GetLocalAddr().GetPort());
    printf("[Client] : Remote IP : %s, Port : %u\n",
           IPv4ToStr(Client.GetRemoteAddr().GetIP()).c_str(),
           Client.GetRemoteAddr().GetPort());

    // Empty receive test
    Client.SetNonblockMode();
    memset(buf, 0, sizeof(buf));
    res = Client.Receive(buf, sizeof(buf));
    printf("[Client] : Recv (Empty Test) : %d \"%s\"\n", res, buf);
    Client.SetBlockMode();

    // Send
    res = Client.Send("Client Message", sizeof("client Message"));
    printf("[Client] : Send : %d\n", res);

    // Receive
    memset(buf, 0, sizeof(buf));
    res = Client.Receive(buf, sizeof(buf));
    systime_sleep(1000);
    printf("[Client] : Recv : %d \"%s\"\n", res, buf);

    Client.Close();

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
    thrd_t   ThrdServer, ThrdClient;
    int      ThrdRes;

    assert( thrd_success == thrd_create(&ThrdServer, (thrd_start_t)ServerProc, &ServerPort) );
    assert( thrd_success == thrd_create(&ThrdClient, (thrd_start_t)ClientProc, &ServerPort) );

    assert( thrd_success == thrd_join(ThrdServer, &ThrdRes) );
    assert( thrd_success == thrd_join(ThrdClient, &ThrdRes) );

    return 0;
}
//------------------------------------------------------------------------------

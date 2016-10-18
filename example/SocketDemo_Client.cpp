/**
 * @file
 * @example SockDemo_Client.cpp
 *
 * This is an example of socket application (client version).
 *
 * @code
 */
#include <stdlib.h>
#include <iostream>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <gen/systime.h>
#include <gen/net/socktcp.h>

using namespace std;

//------------------------------------------------------------------------------
string ReceiveString(TSocketTCP* Socket, unsigned Timeout/*Seconds*/)
{
    char     Buffer[2048]   = {0};
    char    *RecPos         = Buffer;
    size_t   SizeRest       = sizeof(Buffer) - 1;
    unsigned TimeStart      = systime_get_clock_count();
    bool     HaveFullString = false;

    while(( !HaveFullString )&&( systime_get_clock_count() - TimeStart < 1000*Timeout ))
    {
        int SizeRec = Socket->Receive(RecPos, SizeRest);
        if( SizeRec < 0 ) continue;

        SizeRest -= SizeRec;
        while( SizeRec-- )
        {
            if( !(*RecPos++) ) HaveFullString = true;
        }
    }

    return Buffer;
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    try
    {
        static const unsigned Timeout = 10;
        ipv4_t     ServerIP;
        uint16_t   ServerPort;
        TSocketTCP Socket;

        if( argc < 3 )
            throw string("Use this program with 2 parameters : server IP and server port.");

        ServerIP   = ipv4_from_str(argv[1]);
        ServerPort = atoi(argv[2]);

        if( !Socket.Connect(TSocketAddr(ServerIP, ServerPort)) )
            throw string("Connect server failed!");
        if( !Socket.Send("test", sizeof("test")) )
            throw string("Message sending failed!");

        while( true )
        {
            string Request;
            string Response;

            cout << "Input message : ";
            getline(cin, Request);

            if( !Socket.Connect(TSocketAddr(ServerIP, ServerPort)) )
                throw string("Connect server failed!");
            if( !Socket.Send(Request.c_str(), Request.length()+1) )
                throw string("Message sending failed!");

            Response = ReceiveString(&Socket, Timeout);
            if( Response.empty() )
                throw string("Server timeout!");
            cout << "Server response : " << Response << endl;

            if( Response == "Goodbye!" ) break;
        }
    }
    catch(string &E)
    {
        cerr << "Error : " << E << endl;
        return 1;
    }

    return 0;
}
//------------------------------------------------------------------------------

/**
 * @endcode
 */

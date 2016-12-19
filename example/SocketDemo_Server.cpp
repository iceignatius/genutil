/**
 * @file
 * @example SockDemo_Server.cpp
 *
 * This is an example of socket application (server version).
 *
 * @code
 */
#include <iostream>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <gen/systime.h>
#include <gen/net/socktcp.h>

using namespace std;

//------------------------------------------------------------------------------
string LookupResponseMessage(const string &Request)
{
    if( Request == "shutdown"     ) return "Goodbye!";
    if( Request == "exit"         ) return "Goodbye!";
    if( Request == "nothing"      ) return "OK, I understand.";
    if( Request == "hello"        ) return "Hi!";
    if( Request == "how are you?" ) return "I am fine, and you?";
    if( Request == "thank you"    ) return "You are welcome.";

    return Request;
}
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
        size_t SizeRec = Socket->Receive(RecPos, SizeRest);
        SizeRest -= SizeRec;

        while( SizeRec-- )
        {
            if( !(*RecPos++) ) HaveFullString = true;
        }

        systime_sleep_awhile();
    }

    return Buffer;
}
//------------------------------------------------------------------------------
void ShowNewConnection(const TSocketTCP* Socket)
{
    cout << "New connection :" << endl;
    cout << "  IP   : " << IPv4ToStr(Socket->GetRemoteAddr().GetIP()) << endl;
    cout << "  Port : " << Socket->GetRemoteAddr().GetPort() << endl;
}
//------------------------------------------------------------------------------
bool ProcessNewConnection(TSocketTCP* Socket, unsigned Timeout)
{
    string Request  = ReceiveString(Socket, Timeout);
    string Response = LookupResponseMessage(Request);

    if( Request.empty() )
    {
        cout << "  Client timeout!" << endl;
        return false;
    }

    cout << "  Client say : [" << Request  << "]" << endl;
    cout << "  Response   : [" << Response << "]" << endl;
    Socket->Send(Response.c_str(), Response.length()+1);

    return Request == "shutdown";
}
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    try
    {
        static const unsigned Timeout = 4;
        TSocketTCP Socket;
        bool       Terminate = false;

        if( !Socket.Listen(TSocketAddr(Socket::AnyIP, Socket::AnyPort), false) )
            throw string("Cannot start server");

        cout << "Server started, port : " << Socket.GetLocalAddr().GetPort() << endl;

        while( !Terminate )
        {
            TSocketTCP *Peer = Socket.GetNewConnect();
            if( Peer )
            {
                ShowNewConnection(Peer);

                Terminate |= ProcessNewConnection(Peer, Timeout);

                cout << "  Close client." << endl;
                Peer->Release();
            }

            systime_sleep_awhile();
        }

        static const char MsgShutdown[] = "Server shutting down";
        cout << MsgShutdown << endl;
        Socket.Close();
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

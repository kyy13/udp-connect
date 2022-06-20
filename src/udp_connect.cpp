// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"

#include <cstring>
#include <ws2tcpip.h>

UdcServer* udcCreateServer(uint16_t port)
{
    UdcServer* server;

    try
    {
        server = new UdcServer;
    }
    catch(...)
    {
        return nullptr;
    }

    if (!server->socket.bind(port, port + 1))
    {
        delete server;
        return nullptr;
    }

    return server;
}

void udcDeleteServer(UdcServer* server)
{
    delete server;
}

//bool udcTryConnect(
//    UdcServer* server,
//    const char* ipString,
//    const char* portString,
//    uint32_t timeout,
//    uint32_t attempts,
//    UdcServiceId& clientId)
//{
//
//}

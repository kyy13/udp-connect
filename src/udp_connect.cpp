// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"
#include "UdcDeviceId.h"
#include "UdcConnection.h"

#include <cstring>
#include <ws2tcpip.h>
#include <chrono>

bool clientSend(const UdcServer* server, const UdcClientInfo& clientInfo, const std::vector<uint8_t>& message)
{
    if (clientInfo.addressFamily == UDC_IPV6)
    {
        // Send connection request
        if (server->socket.send(clientInfo.addressIPv6, clientInfo.port, server->messageBuffer))
        {
            return true;
        }
    }
    else // UDC_IPV4
    {
        // Send connection request
        if (server->socket.send(clientInfo.addressIPv4, clientInfo.port, server->messageBuffer))
        {
            return true;
        }
    }

    return false;
}

UdcServer* udcCreateServer(uint32_t signature, uint16_t portIPv4, uint16_t portIPv6)
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

    if (!server->socket.bind(portIPv4, portIPv6))
    {
        delete server;
        return nullptr;
    }

    server->signature = signature;
    return server;
}

void udcDeleteServer(UdcServer* server)
{
    delete server;
}

bool udcTryConnect(
    UdcServer* server,
    const char* ipString,
    const char* portString,
    uint32_t timeout)
{
    if (server == nullptr || timeout == 0)
    {
        return false;
    }

    auto clientInfo = std::make_unique<UdcClientInfo>();

    // Get address
    // Create preliminary device ID
    if (UdcSocket::stringToIPv6(ipString, portString, clientInfo->addressIPv6, clientInfo->port))
    {
        clientInfo->addressFamily = UDC_IPV6;
        clientInfo->id = newDeviceId(clientInfo->addressIPv6, clientInfo->port);
    }
    else if (UdcSocket::stringToIPv4(ipString, portString, clientInfo->addressIPv4, clientInfo->port))
    {
        clientInfo->addressFamily = UDC_IPV4;
        clientInfo->id = newDeviceId(clientInfo->addressIPv4, clientInfo->port);
    }
    else
    {
        return false;
    }

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    clientInfo->timeout = std::chrono::milliseconds {timeout};
    clientInfo->tryConnectTime = time;
    clientInfo->lastSendTime = time;

    // Add to pending
    server->pendingClients.push(std::move(clientInfo));

    return true;
}

uint32_t udcGetConnectionCount(UdcServer* server)
{
    return server->clients.size();
}

template<class T>
uint8_t* udcReceiveT(UdcServer* server, UdcDeviceId& clientId, uint32_t& size)
{
    UdcMessageId msgId;
    uint16_t port;
    T address;

    while (server->socket.receive(address, port, server->messageBuffer))
    {
        // Read message header
        if (!udcReadHeader(server->messageBuffer, server->signature, msgId))
        {
            continue;
        }

        // Handle message
        switch(msgId)
        {
            case UDC_MSG_CONNECTION_REQUEST:
                tryReadConnectionRequest(server, address, port);
                break;
            case UDC_MSG_CONNECTION_HANDSHAKE:
                tryReadConnectionHandshake(server, address);
                break;
            case UDC_MSG_EXTERNAL:
                return nullptr; // TODO
        }
    }

    return nullptr;
}

uint8_t* udcReceive(UdcServer* server, UdcDeviceId& clientId, uint32_t& size)
{
    // Send outgoing pending connection requests
    if (!server->pendingClients.empty())
    {
        trySendConnectionRequest(server, *server->pendingClients.front());
    }

    // Receive IPv6

    uint8_t* result = udcReceiveT<UdcAddressIPv6>(server, clientId, size);

    if (result != nullptr)
    {
        return result;
    }

    // Receive IPv4

    return udcReceiveT<UdcAddressIPv4>(server, clientId, size);
}

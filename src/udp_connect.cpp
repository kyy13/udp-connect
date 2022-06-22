// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"
#include "UdcDeviceId.h"
#include "UdcConnection.h"
#include "UdcEvent.h"

#include <cstring>
#include <chrono>

UdcServer* udcCreateServer(
    uint32_t signature,
    uint16_t portIPv6,
    uint16_t portIPv4,
    const char* logFileName)
{
    UdcServer* server;

    try
    {
        server = (logFileName == nullptr)
            ? new UdcServer()
            : new UdcServer(logFileName);
    }
    catch(...)
    {
        return nullptr;
    }

    if (!server->socket.bind(portIPv6, portIPv4))
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
    const char* nodeName,
    const char* serviceName,
    uint32_t timeout)
{
    if (server == nullptr || timeout == 0)
    {
        return false;
    }

    auto clientInfo = std::make_unique<UdcClientInfo>();

    clientInfo->nodeName = nodeName;
    clientInfo->serviceName = serviceName;

    // Get address
    // Create preliminary device ID
    if (UdcSocket::stringToIPv6(nodeName, serviceName, clientInfo->addressIPv6, clientInfo->port))
    {
        clientInfo->addressFamily = UDC_IPV6;
        clientInfo->id = newDeviceId(clientInfo->addressIPv6, clientInfo->port);
    }
    else if (UdcSocket::stringToIPv4(nodeName, serviceName, clientInfo->addressIPv4, clientInfo->port))
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

template<class T>
bool udcReceiveMessage(UdcServer* server, UdcEvent& event)
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
                return false;
            case UDC_MSG_CONNECTION_HANDSHAKE:
                return tryReadConnectionHandshake(server, address, event);
            case UDC_MSG_EXTERNAL:
                return false;
        }
    }

    return false;
}

UdcEvent* udcProcessEvents(UdcServer* server)
{
    static UdcEvent event;

    // Send outgoing pending connection requests
    if (!server->pendingClients.empty())
    {
        trySendConnectionRequest(server, *server->pendingClients.front());
    }

    // Receive IPv6
    if (udcReceiveMessage<UdcAddressIPv6>(server, event))
    {
        return &event;
    }

    // Receive IPv4
    if (udcReceiveMessage<UdcAddressIPv4>(server, event))
    {
        return &event;
    }

    return nullptr;
}

UdcEventType udcGetEventType(const UdcEvent* event)
{
    return event->eventType;
}

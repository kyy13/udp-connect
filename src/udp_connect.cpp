// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"
#include "UdcEndPointId.h"
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

    auto clientInfo = std::make_unique<UdcConnection>();

    clientInfo->nodeName = nodeName;
    clientInfo->serviceName = serviceName;

    // Get address
    // Create preliminary device ID
    if (UdcSocket::stringToIPv6(nodeName, serviceName, clientInfo->addressIPv6, clientInfo->port))
    {
        clientInfo->addressFamily = UDC_IPV6;
        clientInfo->id = newEndPointId(clientInfo->addressIPv6, clientInfo->port);
    }
    else if (UdcSocket::stringToIPv4(nodeName, serviceName, clientInfo->addressIPv4, clientInfo->port))
    {
        clientInfo->addressFamily = UDC_IPV4;
        clientInfo->id = newEndPointId(clientInfo->addressIPv4, clientInfo->port);
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

// UDC_MSG_CONNECTION_REQUEST
void trySendConnectionRequest(UdcServer* server, UdcConnection& client)
{
    constexpr auto clientInternalSendTime = std::chrono::milliseconds {200};

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    if ((time - client.lastSendTime) >= clientInternalSendTime)
    {
        // Send connection request
        const UdcMsgConnection msgConnectionRequest =
            {
                .clientId = client.id,
                .serverId = server->id,
            };

        udcGenerateMessage(
            server->messageBuffer,
            msgConnectionRequest,
            server->signature,
            UDC_MSG_CONNECTION_REQUEST);

        if (client.addressFamily == UDC_IPV6)
        {
            server->socket.send(client.addressIPv6, client.port, server->messageBuffer);
        }
        else if (client.addressFamily == UDC_IPV4)
        {
            server->socket.send(client.addressIPv4, client.port, server->messageBuffer);
        }

        client.lastSendTime = time;
    }
}

// UDC_MSG_CONNECTION_REQUEST
template<class T>
void tryReadConnectionRequest(UdcServer* server, const T& address, uint16_t port)
{
    UdcMsgConnection msg;

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return;
    }

    // If this server doesn't know its ID, then grab it from the connection request
    if (isNullEndPointId(server->id))
    {
        server->id = msg.clientId;
    }
    else // Otherwise, send this server's ID
    {
        msg.clientId = server->id;
    }

    // If the client doesn't know its ID, then generate one for it
    if (isNullEndPointId(msg.serverId))
    {
        msg.serverId = newEndPointId(address, port);
    }

    // Generate a handshake response
    udcGenerateMessage(
        server->messageBuffer,
        msg,
        server->signature,
        UDC_MSG_CONNECTION_HANDSHAKE);

    // Send handshake
    server->socket.send(address, port, server->messageBuffer);
}

// UDC_MSG_CONNECTION_HANDSHAKE
template<class T>
bool tryReadConnectionHandshake(UdcServer* server, const T& address, UdcEvent& event)
{
    UdcMsgConnection msg;

    // If there are no pending clients,
    // then ignore
    if (server->pendingClients.empty())
    {
        return false;
    }

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return false;
    }

    // Check for matching server IDs and client IDs
    if (cmpEndPointId(server->id, msg.serverId) &&
        cmpEndPointId(server->pendingClients.front()->id, msg.clientId))
    {
        auto& client = server->pendingClients.front();

        event.eventType = UDC_EVENT_CONNECTION_SUCCESS;
        event.endPointId = msg.clientId;
        event.nodeName = client->nodeName.c_str();
        event.serviceName = client->serviceName.c_str();

        server->clients[msg.clientId] = std::move(client);
        server->pendingClients.pop();

        return true;
    }

    // If this server doesn't know its ID, then grab it from the handshake
    if (isNullEndPointId(server->id))
    {
        server->id = msg.serverId;
    }

    return false;
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

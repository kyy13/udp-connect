// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"
#include "UdcEndPointId.h"

#include <cstring>
#include <chrono>
#include <cassert>
#include <cmath>

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

// UDC_MSG_PING
bool trySendPing(UdcServer* server, UdcConnection& client, UdcEvent& event)
{
    const auto pingPeriod = std::min(
        std::chrono::milliseconds(500),
        client.tryConnectTimeout / 10);

    const auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // Check if enough time has elapsed to send another ping
    const auto deltaPing = currentTime - client.pingPrevTime;
    if (deltaPing >= pingPeriod)
    {
        // Send connection request
        const UdcMsgPingPong msgPingPong =
            {
                .clientId = client.id,
                .timeOnServer = static_cast<uint32_t>(currentTime.count()),
            };

        udcGenerateMessage(
            server->messageBuffer,
            msgPingPong,
            server->signature,
            UDC_MSG_PING);

        if (client.addressFamily == UDC_IPV6)
        {
            server->socket.send(client.addressIPv6, client.port, server->messageBuffer);
        }
        else if (client.addressFamily == UDC_IPV4)
        {
            server->socket.send(client.addressIPv4, client.port, server->messageBuffer);
        }

        client.pingPrevTime = currentTime;
    }

    // Check if enough time has elapsed since last pong to have a lost connection
    const auto deltaPong = currentTime - client.pongPrevTime;
    if (deltaPong >= client.tryConnectTimeout)
    {
        event.eventType = UDC_EVENT_CONNECTION_LOST;
        event.endPointId = client.id;
        client.isConnected = false;
        return true;
    }

    return false;
}

// UDC_MSG_CONNECTION_REQUEST
bool trySendConnectionRequest(UdcServer* server, UdcConnection& client, UdcEvent& event)
{
    assert(!server->pendingClients.empty());

    constexpr auto tryConnectSendPeriod = std::chrono::milliseconds {100};

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // Check if trying to connect is taking too long
    auto totalTime = time - client.tryConnectFirstTime;
    if (totalTime >= client.tryConnectTimeout)
    {
        // Send a timeout event
        event.eventType = UDC_EVENT_CONNECTION_TIMEOUT;
        event.nodeName = client.nodeName.c_str();
        event.serviceName = client.serviceName.c_str();

        // Timed-out, remove pending client from the queue
        server->pendingClients.pop();

        return true;
    }

    // Check if it has been long enough to send another connection request
    auto deltaTime = time - client.tryConnectPrevTime;
    if (deltaTime >= tryConnectSendPeriod)
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

        client.tryConnectPrevTime = time;
    }

    return false;
}

// UDC_MSG_PING
template<class T>
void tryReadPing(UdcServer* server, const T& address, uint16_t port)
{
    UdcMsgPingPong msg;

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return;
    }

    // Generate a pong response
    udcGenerateMessage(
        server->messageBuffer,
        msg,
        server->signature,
        UDC_MSG_PONG);

    // Send pong
    server->socket.send(address, port, server->messageBuffer);
}

// UDC_MSG_PONG
bool tryReadPong(UdcServer* server, UdcEvent& event)
{
    UdcMsgPingPong msg;

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return false;
    }

    auto it = server->clients.find(msg.clientId);

    if (it != server->clients.end())
    {
        auto& client = it->second;

        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        auto pingTime = std::chrono::milliseconds(msg.timeOnServer);

        // check that time is valid
        if (currentTime < pingTime)
        {
            return false;
        }

        // Calculate ping value and set the pong timer
        client->pingValue = currentTime - pingTime;
        client->pongPrevTime = currentTime;

        // If client connection was lost, then the client has now regained connection
        if (!client->isConnected)
        {
            client->isConnected = true;

            event.eventType = UDC_EVENT_CONNECTION_REGAINED;
            event.endPointId = client->id;

            return true;
        }
    }

    return false;
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

        client->isConnected = true;

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
        case UDC_MSG_PING:
            tryReadPing(server, address, port);
            return false;
        case UDC_MSG_PONG:
            return tryReadPong(server, event);
        case UDC_MSG_EXTERNAL:
            return false;
        }
    }

    return false;
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

    clientInfo->isConnected = false;
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

    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    clientInfo->tryConnectTimeout = std::chrono::milliseconds(timeout);
    clientInfo->tryConnectFirstTime = currentTime;
    clientInfo->tryConnectPrevTime = std::chrono::milliseconds(0);

    // Add to pending
    server->pendingClients.push(std::move(clientInfo));

    return true;
}

UdcEvent* udcProcessEvents(UdcServer* server)
{
    // Send outgoing pending connection requests
    if (!server->pendingClients.empty())
    {
        if (trySendConnectionRequest(server, *server->pendingClients.front(), server->event))
        {
            return &server->event;
        }
    }

    // Send pings
    for (auto& pair : server->clients)
    {
        if (trySendPing(server, *pair.second, server->event))
        {
            return &server->event;
        }
    }

    // Receive IPv6
    if (udcReceiveMessage<UdcAddressIPv6>(server, server->event))
    {
        return &server->event;
    }

    // Receive IPv4
    if (udcReceiveMessage<UdcAddressIPv4>(server, server->event))
    {
        return &server->event;
    }

    return nullptr;
}

UdcEventType udcGetEventType(const UdcEvent* event)
{
    return event->eventType;
}

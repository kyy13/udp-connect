// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"

#include <cstring>
#include <chrono>
#include <cassert>
#include <cmath>

UdcServer* udcCreateServer(
    UdcSignature signature,
    uint16_t portIPv6,
    uint16_t portIPv4,
    uint8_t* buffer,
    uint32_t size,
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

    server->idCounter = 0;
    server->signature = signature;
    server->buffer = buffer;
    server->bufferSize = size;

    return server;
}

void udcDeleteServer(UdcServer* server)
{
    delete server;
}

bool udcTryParseAddressIPv4(
    const char* nodeName,
    const char* serviceName,
    UdcAddressIPv4& address,
    uint16_t& port)
{
    return UdcSocket::stringToIPv4(nodeName, serviceName, address, port);
}

bool udcTryParseAddressIPv6(
    const char* nodeName,
    const char* serviceName,
    UdcAddressIPv6& address,
    uint16_t& port)
{
    return UdcSocket::stringToIPv6(nodeName, serviceName, address, port);
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

        uint32_t msgSize = server->bufferSize;

        udcGenerateMessage(
            server->buffer,
            msgSize,
            msgPingPong,
            server->signature,
            UDC_MSG_PING);

        if (client.addressFamily == UDC_IPV6)
        {
            server->socket.send(client.addressIPv6, client.port, server->buffer, msgSize);
        }
        else if (client.addressFamily == UDC_IPV4)
        {
            server->socket.send(client.addressIPv4, client.port, server->buffer, msgSize);
        }

        client.pingPrevTime = currentTime;
    }

    // Check if enough time has elapsed since last pong to have a lost connection
    const auto deltaReceived = currentTime - client.recvPrevTime;
    if (deltaReceived >= client.tryConnectTimeout && client.isConnected)
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

    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // Check if trying to connect is taking too long
    auto totalTime = currentTime - client.tryConnectFirstTime;
    if (totalTime >= client.tryConnectTimeout)
    {
        // Send a timeout event
        event.eventType = UDC_EVENT_CONNECTION_TIMEOUT;
        event.endPointId = client.id;

        // Timed-out, remove pending client from the queue
        server->pendingClients.pop();

        return true;
    }

    // Check if it has been long enough to send another connection request
    auto deltaTime = currentTime - client.tryConnectPrevTime;
    if (deltaTime >= tryConnectSendPeriod)
    {
        // Send connection request
        const UdcMsgConnection msgConnectionRequest =
            {
                .clientId = client.id,
            };

        uint32_t msgSize = server->bufferSize;

        udcGenerateMessage(
            server->buffer,
            msgSize,
            msgConnectionRequest,
            server->signature,
            UDC_MSG_CONNECTION_REQUEST);

        if (client.addressFamily == UDC_IPV6)
        {
            server->socket.send(client.addressIPv6, client.port, server->buffer, msgSize);
        }
        else if (client.addressFamily == UDC_IPV4)
        {
            server->socket.send(client.addressIPv4, client.port, server->buffer, msgSize);
        }

        client.tryConnectPrevTime = currentTime;
    }

    return false;
}

// UDC_MSG_PING
template<class T>
void tryReadPing(UdcServer* server, uint32_t msgSize, const T& address, uint16_t port)
{
    UdcMsgPingPong msg;

    // Parse the message
    if (!udcReadMessage(server->buffer, msgSize, msg))
    {
        return;
    }

//    // Validate that the client is this one
//    if (!cmpEndPointId(msg.clientId, server->id))
//    {
//        // Allow the msg to assign server ID in the case that the server was deleted
//        // and re-instantiated (id reset to 0)
//        if (isNullEndPointId(server->id))
//        {
//            server->id = msg.clientId;
//        }
//        else
//        {
//            return;
//        }
//    }

    msgSize = server->bufferSize;

    // Generate a pong response
    udcGenerateMessage(
        server->buffer,
        msgSize,
        msg,
        server->signature,
        UDC_MSG_PONG);

    // Send pong
    server->socket.send(address, port, server->buffer, msgSize);
}

// UDC_MSG_PONG
bool tryReadPong(UdcServer* server, uint32_t msgSize, UdcEvent& event)
{
    UdcMsgPingPong msg;

    // Parse the message
    if (!udcReadMessage(server->buffer, msgSize, msg))
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

        // Count as message received from client
        client->recvPrevTime = currentTime;

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
void tryReadConnectionRequest(UdcServer* server, uint32_t msgSize, const T& address, uint16_t port)
{
    UdcMsgConnection msg;

    // Parse the message
    if (!udcReadMessage(server->buffer, msgSize, msg))
    {
        return;
    }

//    // If this server doesn't know its ID, then grab it from the connection request
//    if (isNullEndPointId(server->id))
//    {
//        server->id = msg.clientId;
//    }
//    else // Otherwise, send this server's ID
//    {
//        msg.clientId = server->id;
//    }

//    // If the client doesn't know its ID, then generate one for it
//    if (isNullEndPointId(msg.serverId))
//    {
//        msg.serverId = newEndPointId(address, port);
//    }

    // Generate a handshake response
    msgSize = server->bufferSize;

    udcGenerateMessage(
        server->buffer,
        msgSize,
        msg,
        server->signature,
        UDC_MSG_CONNECTION_HANDSHAKE);

    // Send handshake
    server->socket.send(address, port, server->buffer, msgSize);
}

// UDC_MSG_CONNECTION_HANDSHAKE
template<class T>
bool tryReadConnectionHandshake(UdcServer* server, uint32_t msgSize, const T& address, UdcEvent& event)
{
    UdcMsgConnection msg;

    // If there are no pending clients,
    // then ignore
    if (server->pendingClients.empty())
    {
        return false;
    }

    // Parse the message
    if (!udcReadMessage(server->buffer, msgSize, msg))
    {
        return false;
    }

    // Check for matching client ID
    if (server->pendingClients.front()->id == msg.clientId)
    {
        auto& client = server->pendingClients.front();

        event.eventType = UDC_EVENT_CONNECTION_SUCCESS;
        event.endPointId = msg.clientId;

        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        client->isConnected = true;
        client->recvPrevTime = currentTime;

        server->clients[msg.clientId] = std::move(client);
        server->pendingClients.pop();

        return true;
    }

    return false;
}

template<class T>
bool tryReadExternal(UdcServer* server, uint32_t msgSize, const T& address, uint16_t port, UdcEvent& event)
{
    UdcMsgExternal msg;

    if (!udcReadMessage(server->buffer, msgSize, msg))
    {
        return false;
    }

    if constexpr (std::is_same<T, UdcAddressIPv4>::value)
    {
        event.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV4;
        event.addressIPv4 = address;
    }
    else
    {
        event.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV6;
        event.addressIPv6 = address;
    }

    event.endPointId = msg.clientId;
    event.port = port;

    return true;
}

template<class T>
bool udcReceiveMessage(UdcServer* server, UdcEvent& event)
{
    UdcMessageId msgId;
    uint16_t port;
    T address;

    uint32_t msgSize = server->bufferSize;

    while (server->socket.receive(address, port, server->buffer, msgSize))
    {
        // Read message header
        if (udcReadHeader(server->buffer, msgSize, server->signature, msgId))
        {
            switch (msgId)
            {
                case UDC_MSG_CONNECTION_REQUEST:
                    tryReadConnectionRequest(server, msgSize, address, port);
                    break;
                case UDC_MSG_CONNECTION_HANDSHAKE:
                    if (tryReadConnectionHandshake(server, msgSize, address, event))
                    {
                        return true;
                    }
                    break;
                case UDC_MSG_PING:
                    tryReadPing(server, msgSize, address, port);
                    break;
                case UDC_MSG_PONG:
                    if (tryReadPong(server, msgSize, event))
                    {
                        return true;
                    }
                    break;
                case UDC_MSG_EXTERNAL:
                    if (tryReadExternal(server, msgSize, address, port, event))
                    {
                        return true;
                    }
                    break;
            }
        }
    }

    return false;
}

bool udcTryConnect(
    UdcServer* server,
    const char* nodeName,
    const char* serviceName,
    uint32_t timeout,
    UdcEndPointId& endPointId)
{
    uint16_t port;

    union
    {
        UdcAddressIPv4 ipv4;
        UdcAddressIPv6 ipv6;
    };

    if (UdcSocket::stringToIPv6(nodeName, serviceName, ipv6, port))
    {
        return udcTryConnectIPv6(server, ipv6, port, timeout, endPointId);
    }
    else if (UdcSocket::stringToIPv4(nodeName, serviceName, ipv4, port))
    {
        return udcTryConnectIPv4(server, ipv4, port, timeout, endPointId);
    }

    return false;
}

bool udcTryConnectIPv4(
    UdcServer* server,
    const UdcAddressIPv4& ip,
    uint16_t port,
    uint32_t timeout,
    UdcEndPointId& endPointId)
{
    if (server == nullptr || timeout == 0)
    {
        return false;
    }

    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    auto clientInfo = std::make_unique<UdcConnection>();

    clientInfo->id = (server->idCounter++);
    clientInfo->isConnected = false;
    clientInfo->addressFamily = UDC_IPV4;
    clientInfo->addressIPv4 = ip;
    clientInfo->port = port;
    clientInfo->tryConnectTimeout = std::chrono::milliseconds(timeout);
    clientInfo->tryConnectFirstTime = currentTime;
    clientInfo->tryConnectPrevTime = std::chrono::milliseconds(0);

    // Add to pending
    server->pendingClients.push(std::move(clientInfo));

    return true;
}

bool udcTryConnectIPv6(
    UdcServer* server,
    const UdcAddressIPv6& ip,
    uint16_t port,
    uint32_t timeout,
    UdcEndPointId& endPointId)
{
    if (server == nullptr || timeout == 0)
    {
        return false;
    }

    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    auto clientInfo = std::make_unique<UdcConnection>();

    clientInfo->id = (server->idCounter++);
    clientInfo->isConnected = false;
    clientInfo->addressFamily = UDC_IPV6;
    clientInfo->addressIPv6 = ip;
    clientInfo->port = port;
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

bool udcGetResultConnectionEvent(const UdcEvent* event, UdcEndPointId& endPointId)
{
    if (event->eventType > UDC_EVENT_CONNECTION_REGAINED)
    {
        return false;
    }

    endPointId = event->endPointId;
    return true;
}

void udcSendMessage(
    UdcServer* server,
    UdcEndPointId endPointId,
    const uint8_t* data,
    uint32_t size,
    UdcReliability reliability)
{
    auto it = server->clients.find(endPointId);

    if (it == server->clients.end())
    {
        return;
    }

    auto& client = *(it->second);

    if (!client.isConnected)
    {
        return;
    }

    if (client.addressFamily == UDC_IPV6)
    {
        server->socket.send(client.addressIPv6, client.port, data, size);
    }
    else
    {
        server->socket.send(client.addressIPv4, client.port, data, size);
    }
}

bool udcGetResultExternalIPv4Event(const UdcEvent* event, UdcEndPointId& endPointId, UdcAddressIPv4& address, uint16_t& port, uint8_t* buffer, uint32_t& size)
{
//    if (event->eventType != UDC_EVENT_RECEIVE_MESSAGE_IPV4)
//    {
//        return false;
//    }
//
//    endPointId = event->endPointId;
//    address = event->addressIPv4;
//    port = event->port;
//    return true;
    return false;
}

bool udcGetResultExternalIPv6Event(const UdcEvent* event, UdcEndPointId& endPointId, UdcAddressIPv6& address, uint16_t& port, uint8_t* buffer, uint32_t& size)
{
    return false;
}

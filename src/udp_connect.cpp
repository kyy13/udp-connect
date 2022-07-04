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

// UDC_MSG_CONNECTION_REQUEST
//bool trySendConnectionRequest(UdcServer* server, UdcClient& client, UdcEvent& event)
//{
//    assert(!server->pendingClients.empty());
//
//    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
//        std::chrono::system_clock::now().time_since_epoch());
//
//    // Check if trying to connect is taking too long
//    if (client.needsConnectionTimeoutEvent(currentTime))
//    {
//        // Send a timeout event
//        event.eventType = UDC_EVENT_CONNECTION_TIMEOUT;
//        event.endPointId = client.id();
//
//        // Timed-out, remove pending client from the queue
//        server->pendingClients.pop();
//
//        return true;
//    }
//
//    // Check if it has been long enough to send another connection request
//    if (client.needsConnectionAttempt(currentTime))
//    {
//        client.retryConnecting(currentTime);
//
//        // Send connection request
//        assert(server->bufferSize >= serial::msgConnection::SIZE);
//
//        auto& buffer = server->buffer;
//        serial::msgHeader::serializeMsgSignature(buffer, server->signature);
//        serial::msgHeader::serializeMsgId(buffer, UDC_MSG_CONNECTION_REQUEST);
//        serial::msgConnection::serializeEndPointId(buffer, client.id());
//
//        server->socket.send(client.outgoingAddress(), buffer, serial::msgConnection::SIZE);
//    }
//
//    return false;
//}

// UDC_MSG_PING
void tryReadPing(UdcServer* server, uint32_t msgSize, const UdcAddressMux& address)
{
    if (msgSize != serial::msgPingPong::SIZE)
    {
        return;
    }

    // Change the message ID UDC_MSG_PONG to UDC_MSG_PONG
    // everything else can stay the same
    auto& buffer = server->buffer;
    serial::msgHeader::serializeMsgId(buffer, UDC_MSG_PONG);

    // Send pong
    server->socket.send(address, server->buffer, serial::msgPingPong::SIZE);
}

// UDC_MSG_PONG
bool tryReadPong(UdcServer* server, uint32_t msgSize, UdcEvent& event)
{
    if (msgSize != serial::msgPingPong::SIZE)
    {
        return false;
    }

    UdcEndPointId endPointId;
    serial::msgPingPong::deserializeEndPointId(server->buffer, endPointId);

    auto it = server->clients.find(endPointId);

    if (it != server->clients.end())
    {
        uint32_t timeStamp;
        serial::msgPingPong::deserializeTimeStamp(server->buffer, timeStamp);

        auto& client = it->second;

        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());

        auto pingTime = std::chrono::milliseconds(timeStamp);

        // check that time is valid
        if (currentTime < pingTime)
        {
            return false;
        }

        // If client connection was lost, then the client has now regained connection
        if (client->receivePong(pingTime, currentTime))
        {
            event.eventType = UDC_EVENT_CONNECTION_REGAINED;
            event.endPointId = client->id();

            return true;
        }
    }

    return false;
}

// UDC_MSG_CONNECTION_REQUEST
void tryReadConnectionRequest(UdcServer* server, uint32_t msgSize, const UdcAddressMux& address)
{
    if (msgSize != serial::msgConnection::SIZE)
    {
        return;
    }

    // Change message ID from UDC_CONNECTION_REQUEST to UDC_MSG_CONNECTION_HANDSHAKE
    // nothing else needs to change
    serial::msgHeader::serializeMsgId(server->buffer, UDC_MSG_CONNECTION_HANDSHAKE);

    // Send handshake
    server->socket.send(address, server->buffer, msgSize);
}

// UDC_MSG_CONNECTION_HANDSHAKE
bool tryReadConnectionHandshake(UdcServer* server, uint32_t msgSize, const UdcAddressMux& address, UdcEvent& event)
{
    // If there are no pending clients,
    // then ignore
    if (server->pendingClients.empty())
    {
        return false;
    }

    if (msgSize != serial::msgConnection::SIZE)
    {
        return false;
    }

    UdcEndPointId endPointId;
    serial::msgConnection::deserializeEndPointId(server->buffer, endPointId);

    // Check for matching client ID
    if (server->pendingClients.front()->id() != endPointId)
    {
        return false;
    }

    auto& client = server->pendingClients.front();

    event.eventType = UDC_EVENT_CONNECTION_SUCCESS;
    event.endPointId = endPointId;

    auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    client->receiveHandshake(currentTime);

    server->clients[endPointId] = std::move(client);
    server->pendingClients.pop();

    return true;
}

bool tryReadUnreliable(UdcServer* server, uint32_t msgSize, const UdcAddressMux& address, UdcEvent& event)
{
    if (msgSize <= serial::msgHeader::SIZE)
    {
        return false;
    }

    if (address.family == UDC_IPV4)
    {
        event.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV4;
        event.addressIPv4 = address.address.ipv4;
    }
    else
    {
        event.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV6;
        event.addressIPv6 = address.address.ipv6;
    }

    event.port = address.port;
    event.msgIndex = serial::msgHeader::SIZE;
    event.msgSize = msgSize - serial::msgHeader::SIZE;

    return true;
}

bool udcReceiveMessage(UdcServer* server, UdcEvent& event)
{
    UdcSignature signature;
    UdcMessageId msgId;
    UdcAddressMux address;

    auto& buffer = server->buffer;
    uint32_t msgSize = server->bufferSize;

    while (server->socket.receive(address, buffer, msgSize))
    {
        // Read message header

        if (msgSize < serial::msgHeader::SIZE)
        {
            continue;
        }

        serial::msgHeader::deserializeMsgSignature(buffer, signature);

        if (memcmp(server->signature.bytes, signature.bytes, sizeof(signature.bytes)) != 0)
        {
            continue;
        }

        serial::msgHeader::deserializeMsgId(buffer, msgId);

        switch (msgId)
        {
            case UDC_MSG_CONNECTION_REQUEST:
                tryReadConnectionRequest(server, msgSize, address);
                break;
            case UDC_MSG_CONNECTION_HANDSHAKE:
                if (tryReadConnectionHandshake(server, msgSize, address, event))
                {
                    return true;
                }
                break;
            case UDC_MSG_PING:
                tryReadPing(server, msgSize, address);
                break;
            case UDC_MSG_PONG:
                if (tryReadPong(server, msgSize, event))
                {
                    return true;
                }
                break;
            case UDC_MSG_UNRELIABLE:
                if (tryReadUnreliable(server, msgSize, address, event))
                {
                    return true;
                }
                break;
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

    UdcAddressMux address =
        {
            .family = UDC_IPV4,
            .address = {.ipv4 = ip},
            .port = port,
        };

    auto clientInfo = std::make_unique<UdcClient>(
        (server->idCounter++),
        address,
        std::chrono::milliseconds(timeout),
        std::min(std::chrono::milliseconds(500),
        std::chrono::milliseconds(timeout) / 10));

    clientInfo->startConnecting(currentTime);

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

    UdcAddressMux address =
        {
            .family = UDC_IPV6,
            .address = {.ipv6 = ip},
            .port = port,
        };

    auto clientInfo = std::make_unique<UdcClient>(
        (server->idCounter++),
        address,
        std::chrono::milliseconds(timeout),
        std::min(std::chrono::milliseconds(500),
        std::chrono::milliseconds(timeout) / 10));

    clientInfo->startConnecting(currentTime);

    // Add to pending
    server->pendingClients.push(std::move(clientInfo));

    return true;
}

UdcEvent* udcProcessEvents(UdcServer* server)
{
    const auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // Send outgoing pending connection requests
    if (!server->pendingClients.empty())
    {
        auto& client = *server->pendingClients.front();
        auto& event = server->event;

        // Check if trying to connect is taking too long
        if (client.needsConnectionTimeoutEvent(currentTime))
        {
            // Send a timeout event
            event.eventType = UDC_EVENT_CONNECTION_TIMEOUT;
            event.endPointId = client.id();

            // Timed-out, remove pending client from the queue
            server->pendingClients.pop();

            return &event;
        }

        // Check if it has been long enough to send another connection request
        if (client.needsConnectionAttempt(currentTime))
        {
            client.retryConnecting(currentTime);

            // Send connection request
            assert(server->bufferSize >= serial::msgConnection::SIZE);

            auto& buffer = server->buffer;
            serial::msgHeader::serializeMsgSignature(buffer, server->signature);
            serial::msgHeader::serializeMsgId(buffer, UDC_MSG_CONNECTION_REQUEST);
            serial::msgConnection::serializeEndPointId(buffer, client.id());

            server->socket.send(client.outgoingAddress(), buffer, serial::msgConnection::SIZE);
        }
    }

    // Send pings
    for (auto& pair : server->clients)
    {
        auto& client = *pair.second;
        auto& event = server->event;

        // Send PING
        if (client.needsPing(currentTime))
        {
            assert(server->bufferSize >= serial::msgPingPong::SIZE);
            auto& buffer = server->buffer;

            serial::msgHeader::serializeMsgSignature(buffer, server->signature);
            serial::msgHeader::serializeMsgId(buffer, UDC_MSG_PING);
            serial::msgPingPong::serializeEndPointId(buffer, client.id());
            serial::msgPingPong::serializeTimeStamp(buffer, currentTime.count());

            server->socket.send(client.outgoingAddress(), buffer, serial::msgPingPong::SIZE);
        }

        // Throw connection lost event if needed
        if (client.needsConnectionLostEvent(currentTime))
        {
            client.setConnectionLost();

            event.eventType = UDC_EVENT_CONNECTION_LOST;
            event.endPointId = client.id();

            return &event;
        }
    }

    // Receive messages
    if (udcReceiveMessage(server, server->event))
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

    if (!client.connected())
    {
        return;
    }

    auto& buffer = server->buffer;

    serial::msgHeader::serializeMsgSignature(buffer, server->signature);
    serial::msgHeader::serializeMsgId(buffer, UDC_MSG_UNRELIABLE);
    serial::msgUnreliable::serializeData(buffer, data, size);

    server->socket.send(client.outgoingAddress(), buffer, serial::msgHeader::SIZE + size);
}

bool udcGetResultExternalIPv4Event(const UdcEvent* event, UdcAddressIPv4& address, uint16_t& port, uint32_t& msgIndex, uint32_t& msgSize)
{
    if (event->eventType != UDC_EVENT_RECEIVE_MESSAGE_IPV4)
    {
        return false;
    }

    address = event->addressIPv4;
    port = event->port;
    msgIndex = event->msgIndex;
    msgSize = event->msgSize;

    return true;
}

bool udcGetResultExternalIPv6Event(const UdcEvent* event, UdcAddressIPv6& address, uint16_t& port, uint32_t& msgIndex, uint32_t& msgSize)
{
    if (event->eventType != UDC_EVENT_RECEIVE_MESSAGE_IPV6)
    {
        return false;
    }

    address = event->addressIPv6;
    port = event->port;
    msgIndex = event->msgIndex;
    msgSize = event->msgSize;

    return true;
}

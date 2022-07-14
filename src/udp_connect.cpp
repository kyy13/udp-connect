// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include "UdcServer.h"
#include "UdcMessage.h"

#include <cstring>
#include <chrono>
#include <cassert>
#include <cmath>

uint32_t udcGetMinimumBufferSize()
{
    return serial::msgHeader::SIZE + sizeof(uint32_t);
}

UdcServer* udcCreateServer(
    UdcSignature signature,
    uint16_t portIPv6,
    uint16_t portIPv4,
    uint8_t* buffer,
    uint32_t size,
    const char* logFileName)
{
    if (size < udcGetMinimumBufferSize())
    {
        return nullptr;
    }

    UdcServerImpl* server;

    try
    {
        server = (logFileName == nullptr)
            ? new UdcServerImpl(signature, portIPv4, portIPv6, buffer, size)
            : new UdcServerImpl(signature, portIPv4, portIPv6, buffer, size, logFileName);
    }
    catch(...)
    {
        return nullptr;
    }

    return reinterpret_cast<UdcServer*>(server);
}

void udcDeleteServer(UdcServer* server)
{
    delete reinterpret_cast<UdcServerImpl*>(server);
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

    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);

    UdcAddressMux address =
        {
            .family = UDC_IPV4,
            .address = {.ipv4 = ip},
            .port = port,
        };

    endPointId = serverImpl->createUniqueId();

    auto client = std::make_shared<UdcClient>(
        endPointId,
        address,
        std::chrono::milliseconds(timeout),
        std::min(std::chrono::milliseconds(500),
        std::chrono::milliseconds(timeout) / 10));

    // Add to pending
    serverImpl->addPendingClient(std::move(client), currentTime);

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

    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);

    UdcAddressMux address =
        {
            .family = UDC_IPV6,
            .address = {.ipv6 = ip},
            .port = port,
        };

    endPointId = serverImpl->createUniqueId();

    auto client = std::make_shared<UdcClient>(
        endPointId,
        address,
        std::chrono::milliseconds(timeout),
        std::min(std::chrono::milliseconds(500),
        std::chrono::milliseconds(timeout) / 10));

    // Add to pending
    serverImpl->addPendingClient(std::move(client), currentTime);

    return true;
}

bool udcGetStatus(UdcServer* server, UdcEndPointId id, uint32_t& ping)
{
    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);

    std::chrono::milliseconds cping;

    bool result = serverImpl->getEndPointStatus(id, cping);
    ping = cping.count();

    return result;
}

void udcDisconnect(UdcServer* server, UdcEndPointId endPointId)
{
    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);
    serverImpl->disconnectFromClient(endPointId);
}

const UdcEvent* udcProcessEvents(UdcServer* server)
{
    const auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);

    // Send outgoing pending connection requests
    auto* event = serverImpl->updatePendingClients(currentTime);

    if (event != nullptr)
    {
        return event;
    }

    // Send pings and get connection status
    event = serverImpl->updateClientConnectionStatus(currentTime);

    if (event != nullptr)
    {
        return event;
    }

    // Receive messages
    return serverImpl->receiveMessages(currentTime);
}

UdcEventType udcGetEventType(const UdcEvent* event)
{
    return event->eventType;
}

bool udcSendMessage(
    UdcServer* server,
    UdcEndPointId endPointId,
    const uint8_t* data,
    uint32_t size,
    UdcMessageType reliability)
{
    auto* serverImpl = reinterpret_cast<UdcServerImpl*>(server);

    return (reliability == UDC_UNRELIABLE_MESSAGE)
        ? serverImpl->sendUnreliableMessage(endPointId, data, size)
        : serverImpl->sendReliableMessage(endPointId, data, size);
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

bool udcGetResultMessageIPv4Event(const UdcEvent* event, UdcAddressIPv4& address, uint16_t& port, uint32_t& msgIndex, uint32_t& msgSize)
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

bool udcGetResultMessageIPv6Event(const UdcEvent* event, UdcAddressIPv6& address, uint16_t& port, uint32_t& msgIndex, uint32_t& msgSize)
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

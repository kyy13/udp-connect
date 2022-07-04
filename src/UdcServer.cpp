// udp-connect
// Kyle J Burgess

#include "UdcServer.h"
#include "UdcMessage.h"

#include <stdexcept>
#include <cassert>

UdcServerImpl::UdcServerImpl(UdcSignature signature, uint16_t portIPv4, uint16_t portIPv6, uint8_t* buffer, uint32_t bufferSize)
    : m_idCounter(0)
    , m_packetSignature(signature)
    , m_eventBuffer({})
    , m_messageBuffer(buffer)
    , m_messageBufferSize(bufferSize)
{
    if (!m_socket.bind(portIPv6, portIPv4))
    {
        throw std::runtime_error("could not bind to port");
    }
}

UdcServerImpl::UdcServerImpl(UdcSignature signature, uint16_t portIPv4, uint16_t portIPv6, uint8_t* buffer, uint32_t bufferSize, const std::string& logFileName)
    : m_socket(logFileName)
    , m_idCounter(0)
    , m_packetSignature(signature)
    , m_eventBuffer({})
    , m_messageBuffer(buffer)
    , m_messageBufferSize(bufferSize)
{
    if (!m_socket.bind(portIPv6, portIPv4))
    {
        throw std::runtime_error("could not bind to port");
    }
}

UdcEndPointId UdcServerImpl::createUniqueId()
{
    ++m_idCounter;
    return m_idCounter;
}

void UdcServerImpl::addPendingClient(std::unique_ptr<UdcClient> client, std::chrono::milliseconds time)
{
    client->startConnecting(time);
    m_pendingClients.push(std::move(client));
}

void UdcServerImpl::sendUnreliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size)
{
    UdcClient* client;
    if (!tryGetClient(endPointId, &client))
    {
        return;
    }

    if (!client->connected())
    {
        return;
    }

    serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
    serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_UNRELIABLE);
    serial::msgUnreliable::serializeData(m_messageBuffer, data, size);

    m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgHeader::SIZE + size);
}

const UdcEvent* UdcServerImpl::receiveMessages(std::chrono::milliseconds time)
{
    UdcSignature signature;
    UdcMessageId msgId;
    UdcAddressMux address;

    uint32_t msgSize = m_messageBufferSize;

    while (m_socket.receive(address, m_messageBuffer, msgSize))
    {
        // Read message header
        if (msgSize < serial::msgHeader::SIZE)
        {
            continue;
        }

        serial::msgHeader::deserializeMsgSignature(m_messageBuffer, signature);

        if (memcmp(m_packetSignature.bytes, signature.bytes, sizeof(signature.bytes)) != 0)
        {
            continue;
        }

        serial::msgHeader::deserializeMsgId(m_messageBuffer, msgId);

        switch (msgId)
        {
            case UDC_MSG_CONNECTION_REQUEST:
                if (msgSize == serial::msgConnection::SIZE)
                {
                    processConnectionRequest(address);
                }
                break;
            case UDC_MSG_CONNECTION_HANDSHAKE:
                if (msgSize == serial::msgConnection::SIZE)
                {
                    auto event = processConnectionHandshake(address, time);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_PING:
                if (msgSize == serial::msgPingPong::SIZE)
                {
                    processPing(address);
                }
                break;
            case UDC_MSG_PONG:
                if (msgSize == serial::msgPingPong::SIZE)
                {
                    auto event = processPong(address, time);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_UNRELIABLE:
                if (msgSize > serial::msgHeader::SIZE)
                {
                    auto event = processUnreliable(address, msgSize);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            default:
                break;
        }
    }

    return nullptr;
}

const UdcEvent* UdcServerImpl::updatePendingClients(std::chrono::milliseconds time)
{
    // Get first pending client
    UdcClient* client;
    if (!tryGetFirstPendingClient(&client))
    {
        return nullptr;
    }

    // Check if trying to connect is taking too long
    if (client->needsConnectionTimeoutEvent(time))
    {
        // Send a timeout event
        m_eventBuffer.eventType = UDC_EVENT_CONNECTION_TIMEOUT;
        m_eventBuffer.endPointId = client->id();

        // Timed-out, remove pending client from the queue
        m_pendingClients.pop();

        return &m_eventBuffer;
    }

    // Check if it has been long enough to send another connection request
    if (client->needsConnectionAttempt(time))
    {
        client->retryConnecting(time);

        // Send connection request
        assert(m_messageBufferSize >= serial::msgConnection::SIZE);

        serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
        serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_CONNECTION_REQUEST);
        serial::msgConnection::serializeEndPointId(m_messageBuffer, client->id());

        m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgConnection::SIZE);
    }

    return nullptr;
}

const UdcEvent* UdcServerImpl::updateClientConnectionStatus(std::chrono::milliseconds time)
{
    for (auto& pair : m_clients)
    {
        auto* client = pair.second.get();

        // Send PING
        if (client->needsPing(time))
        {
            assert(m_messageBufferSize >= serial::msgPingPong::SIZE);

            serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
            serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_PING);
            serial::msgPingPong::serializeEndPointId(m_messageBuffer, client->id());
            serial::msgPingPong::serializeTimeStamp(m_messageBuffer, time.count());

            m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgPingPong::SIZE);
        }

        // Throw connection lost event if needed
        if (client->needsConnectionLostEvent(time))
        {
            client->setConnectionLost();

            m_eventBuffer.eventType = UDC_EVENT_CONNECTION_LOST;
            m_eventBuffer.endPointId = client->id();

            return &m_eventBuffer;
        }
    }

    return nullptr;
}

void UdcServerImpl::processConnectionRequest(const UdcAddressMux& fromAddress)
{
    // Change message ID from UDC_CONNECTION_REQUEST to UDC_MSG_CONNECTION_HANDSHAKE
    // nothing else needs to change
    serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_CONNECTION_HANDSHAKE);

    // Send handshake
    m_socket.send(fromAddress, m_messageBuffer, serial::msgConnection::SIZE);
}

const UdcEvent* UdcServerImpl::processConnectionHandshake(const UdcAddressMux& fromAddress, std::chrono::milliseconds time)
{
    UdcClient* client;

    // Get client pending connection
    if (!tryGetFirstPendingClient(&client))
    {
        return nullptr;
    }

    // Get endpoint id from message
    UdcEndPointId endPointId;
    serial::msgConnection::deserializeEndPointId(m_messageBuffer, endPointId);

    if (client->id() != endPointId)
    {
        return nullptr;
    }

    // Verify connection
    client->receiveHandshake(time);
    m_clients[endPointId] = std::move(m_pendingClients.front());
    m_pendingClients.pop();

    m_eventBuffer.eventType = UDC_EVENT_CONNECTION_SUCCESS;
    m_eventBuffer.endPointId = endPointId;
    return &m_eventBuffer;
}

void UdcServerImpl::processPing(const UdcAddressMux& fromAddress)
{
    // Change the message ID UDC_MSG_PING to UDC_MSG_PONG
    // everything else can stay the same
    serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_PONG);

    // Send pong
    m_socket.send(fromAddress, m_messageBuffer, serial::msgPingPong::SIZE);
}

const UdcEvent* UdcServerImpl::processPong(const UdcAddressMux& fromAddress, std::chrono::milliseconds time)
{
    // Get endpoint id
    UdcEndPointId endPointId;
    serial::msgPingPong::deserializeEndPointId(m_messageBuffer, endPointId);

    // Check if endpoint id is a client
    UdcClient* client;
    if (!tryGetClient(endPointId, &client))
    {
        return nullptr;
    }

    // Get timestamp
    uint32_t timeStamp;
    serial::msgPingPong::deserializeTimeStamp(m_messageBuffer, timeStamp);
    auto timeStampMs = std::chrono::milliseconds(timeStamp);

    if (time < timeStampMs)
    {
        return nullptr;
    }

    // Receive pong on client
    // if true, then a connection has been regained
    if (!client->receivePong(timeStampMs, time))
    {
        return nullptr;
    }

    // Connection has been regained
    m_eventBuffer.eventType = UDC_EVENT_CONNECTION_REGAINED;
    m_eventBuffer.endPointId = client->id();
    return &m_eventBuffer;
}

const UdcEvent* UdcServerImpl::processUnreliable(const UdcAddressMux& fromAddress, uint32_t msgSize)
{
    if (fromAddress.family == UDC_IPV4)
    {
        m_eventBuffer.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV4;
        m_eventBuffer.addressIPv4 = fromAddress.address.ipv4;
    }
    else
    {
        m_eventBuffer.eventType = UDC_EVENT_RECEIVE_MESSAGE_IPV6;
        m_eventBuffer.addressIPv6 = fromAddress.address.ipv6;
    }

    m_eventBuffer.port = fromAddress.port;
    m_eventBuffer.msgIndex = serial::msgHeader::SIZE;
    m_eventBuffer.msgSize = msgSize - serial::msgHeader::SIZE;
    return &m_eventBuffer;
}

bool UdcServerImpl::tryGetClient(UdcEndPointId clientId, UdcClient** client)
{
    auto it = m_clients.find(clientId);

    if (it == m_clients.end())
    {
        return false;
    }

    *client = it->second.get();
    return true;
}

bool UdcServerImpl::tryGetFirstPendingClient(UdcClient** client)
{
    if (m_pendingClients.empty())
    {
        return false;
    }

    *client = m_pendingClients.front().get();
    return true;
}

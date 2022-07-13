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
    // Try to bind socket
    if (!m_socket.bind(portIPv6, portIPv4))
    {
        throw std::runtime_error("could not bind to port");
    }

    // Write message signature into buffer
    // this is needed by send/recv in every message
    // and only needs to be rewritten if receiving message has
    // incorrect signature
    serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
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

    // Write message signature into buffer
    // this is needed by send/recv in every message
    // and only needs to be rewritten if receiving message has
    // incorrect signature
    serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
}

UdcEndPointId UdcServerImpl::createUniqueId()
{
    ++m_idCounter;
    return m_idCounter;
}

void UdcServerImpl::addPendingClient(std::shared_ptr<UdcClient> client, std::chrono::milliseconds time)
{
    client->startConnecting(time);
    m_pendingClients.push_back(std::move(client));
}

void UdcServerImpl::disconnectFromClient(UdcEndPointId endPointId)
{
    // First try to remove from client id and address hash maps
    // if they aren't there, then it could still be in the pending queue
    {
        auto it = m_clientsById.find(endPointId);

        if (it != m_clientsById.end())
        {
            auto* client = it->second.get();

            // Remove client from clients by address
            m_clientsByAddress.erase(client->outgoingAddress());

            // Remove client from clients by id
            m_clientsById.erase(it);

            return;
        }
    }

    // Try to remove client from pending
    for (auto it = m_pendingClients.begin(); it != m_pendingClients.end();)
    {
        auto& client = *it;

        if (client->id() == endPointId)
        {
            it = m_pendingClients.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool UdcServerImpl::sendUnreliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size)
{
    if (size + serial::msgUnreliable::SIZE > m_messageBufferSize)
    {
        return false;
    }

    UdcClient* client;
    if (!tryGetClient(endPointId, &client))
    {
        return false;
    }

    if (!client->connected())
    {
        return true;
    }

    serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_UNRELIABLE);
    serial::msgUnreliable::serializeData(m_messageBuffer, data, size);

    m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgHeader::SIZE + size);
    return true;
}

bool UdcServerImpl::sendReliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size)
{
    if (size + serial::msgReliable::SIZE > m_messageBufferSize)
    {
        return false;
    }

    UdcClient* client;
    if (!tryGetClient(endPointId, &client))
    {
        return false;
    }

    client->reliableMessages().push(std::vector<uint8_t>(data, data + size));
    return true;
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
            // Write message correct signature back into buffer
            serial::msgHeader::serializeMsgSignature(m_messageBuffer, m_packetSignature);
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
                if (msgSize >= serial::msgHeader::SIZE)
                {
                    auto event = processUnreliable(address, msgSize);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_RESET:
                if (msgSize == serial::msgReliable::SIZE)
                {
                    auto event = processReliableMessage(-1, address, msgSize);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_0:
                if (msgSize >= serial::msgReliable::SIZE)
                {
                    auto event = processReliableMessage(0, address, msgSize);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_1:
                if (msgSize >= serial::msgReliable::SIZE)
                {
                    auto event = processReliableMessage(1, address, msgSize);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_HANDSHAKE_RESET:
                if (msgSize == serial::msgReliable::SIZE)
                {
                    auto event = processReliableHandshake(-1, address, time);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_HANDSHAKE_0:
                if (msgSize == serial::msgReliable::SIZE)
                {
                    auto event = processReliableHandshake(0, address, time);

                    if (event != nullptr)
                    {
                        return event;
                    }
                }
                break;
            case UDC_MSG_RELIABLE_HANDSHAKE_1:
                if (msgSize == serial::msgReliable::SIZE)
                {
                    auto event = processReliableHandshake(1, address, time);

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
        m_pendingClients.pop_front();

        return &m_eventBuffer;
    }

    // Check if it has been long enough to send another connection request
    if (client->needsConnectionAttempt(time))
    {
        client->retryConnecting(time);

        // Send connection request
        assert(m_messageBufferSize >= serial::msgConnection::SIZE);

        serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_CONNECTION_REQUEST);
        serial::msgConnection::serializeEndPointId(m_messageBuffer, client->id());

        m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgConnection::SIZE);
    }

    return nullptr;
}

const UdcEvent* UdcServerImpl::updateClientConnectionStatus(std::chrono::milliseconds time)
{
    for (auto& pair : m_clientsById)
    {
        auto* client = pair.second.get();

        // Send reliable messages
        if (!client->reliableMessages().empty())
        {
            int reliableState = client->reliableState();

            if (reliableState == -1)
            {
                assert(m_messageBufferSize >= serial::msgReliable::SIZE);

                serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_RELIABLE_RESET);
                serial::msgReliable::serializeTimeStamp(m_messageBuffer, time.count());

                m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgReliable::SIZE);
            }
            else
            {
                if (client->needsReliableReset(time))
                {
                    client->setReliableState(-1);
                    client->resetSendReliable();
                }
                else
                {
                    auto& msg = client->reliableMessages().front();

                    assert(m_messageBufferSize >= serial::msgReliable::SIZE + msg.size());

                    serial::msgHeader::serializeMsgId(m_messageBuffer, (reliableState == 0)
                        ? UDC_MSG_RELIABLE_0
                        : UDC_MSG_RELIABLE_1);
                    serial::msgReliable::serializeTimeStamp(m_messageBuffer, time.count());
                    serial::msgReliable::serializeData(m_messageBuffer, msg.data(), msg.size());

                    m_socket.send(client->outgoingAddress(), m_messageBuffer, serial::msgReliable::SIZE + msg.size());

                    client->setSendReliable(time);
                }
            }
        }

        // Send PING
        if (client->needsPing(time))
        {
            assert(m_messageBufferSize >= serial::msgPingPong::SIZE);

            serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_PING);
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

    // Check that outgoingAddress isn't already connected
    if (m_clientsByAddress.find(fromAddress) != m_clientsByAddress.cend())
    {
        return nullptr;
    }

    // Complete connection
    client->receiveConnectionHandshake(time);
    m_clientsById[endPointId] = m_pendingClients.front();
    m_clientsByAddress.insert(fromAddress, m_pendingClients.front());
    m_pendingClients.pop_front();

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
    // Check if fromAddress belongs to a client
    UdcClient* client;
    if (!tryGetClient(fromAddress, &client))
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

const UdcEvent* UdcServerImpl::processReliableMessage(int state, const UdcAddressMux& fromAddress, uint32_t msgSize)
{
    auto it = m_reliableStates.find(fromAddress);

    if (state == -1)
    {
        // reset reliable state for address
        if (it != m_reliableStates.end())
        {
            it->second = 0;
        }

        // send handshake
        serial::msgHeader::serializeMsgId(m_messageBuffer, UDC_MSG_RELIABLE_HANDSHAKE_RESET);
        m_socket.send(fromAddress, m_messageBuffer, serial::msgReliable::SIZE);

        return nullptr;
    }

    bool notInHash = (it == m_reliableStates.end());
    bool process = (notInHash || it->second == state);

    // move to next state
    if (notInHash)
    {
        m_reliableStates.insert(fromAddress, (state == 0) ? 1 : 0);
    }
    else
    {
        it->second = (state == 0) ? 1 : 0;
    }

    // send handshake
    serial::msgHeader::serializeMsgId(m_messageBuffer, (state == 0) ? UDC_MSG_RELIABLE_HANDSHAKE_0 : UDC_MSG_RELIABLE_HANDSHAKE_1);
    m_socket.send(fromAddress, m_messageBuffer, serial::msgReliable::SIZE);

    // process message
    if (process)
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
        m_eventBuffer.msgIndex = serial::msgReliable::SIZE;
        m_eventBuffer.msgSize = msgSize - serial::msgReliable::SIZE;

        return &m_eventBuffer;
    }

    return nullptr;
}

const UdcEvent* UdcServerImpl::processReliableHandshake(int state, const UdcAddressMux& fromAddress, std::chrono::milliseconds time)
{
    // Check if fromAddress belongs to a client
    UdcClient* client;
    if (!tryGetClient(fromAddress, &client))
    {
        return nullptr;
    }

    // Get timestamp
    uint32_t timeStamp;
    serial::msgReliable::deserializeTimeStamp(m_messageBuffer, timeStamp);
    auto timeStampMs = std::chrono::milliseconds(timeStamp);

    if (time < timeStampMs)
    {
        return nullptr;
    }

    // Process handshake
    int reliableState = client->reliableState();

    // handshake must match the server's expectation of client state
    if (state == reliableState)
    {
        if (reliableState != -1 && !client->reliableMessages().empty())
        {
            client->reliableMessages().pop();
        }

        // -1 -> 0 (reset), 0 -> 1, 1 -> 0
        client->setReliableState((state == 0) ? 1 : 0);

        // acknowledge received, and await sending a new reliable message
        client->resetSendReliable();
    }

    // Check for lost connection regained from reliable handshake
    if (!client->receiveReliableHandshake(timeStampMs, time))
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
    auto it = m_clientsById.find(clientId);

    if (it == m_clientsById.end())
    {
        return false;
    }

    *client = it->second.get();
    return true;
}

bool UdcServerImpl::tryGetClient(const UdcAddressMux& address, UdcClient** client)
{
    auto it = m_clientsByAddress.find(address);

    if (it == m_clientsByAddress.cend())
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

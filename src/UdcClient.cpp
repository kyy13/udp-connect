// udp-connect
// Kyle J Burgess

#include "UdcClient.h"

UdcClient::UdcClient(
    UdcEndPointId endPointId,
    const UdcAddressMux& outgoingAddress,
    std::chrono::milliseconds pingPeriod,
    std::chrono::milliseconds timeoutPeriod)
    : m_id(endPointId)
    , m_reliableState(0)
    , m_isConnected(false)
    , m_outgoingAddress(outgoingAddress)
    , m_incomingAddress({})
    , m_pingPeriod(pingPeriod)
    , m_connectionTimeoutPeriod(timeoutPeriod)
    , m_connectionLostPeriod(timeoutPeriod)
    , m_connectionAttemptPeriod(pingPeriod)
    , m_reliableTimeoutPeriod(timeoutPeriod)
    , m_ping(0)
    , m_pingLastSetTime(0)
    , m_lastReceivedTime(0)
    , m_reliableSentTime(0)
    , m_firstConnectAttemptTime(0)
    , m_prevConnectAttemptTime(0)
{}

UdcEndPointId UdcClient::id() const
{
    return m_id;
}

int UdcClient::reliableState() const
{
    return m_reliableState;
}

void UdcClient::setReliableState(int state)
{
    m_reliableState = state;
}

std::queue<std::vector<uint8_t>>& UdcClient::reliableMessages()
{
    return m_reliableMessages;
}

bool UdcClient::connected() const
{
    return m_isConnected;
}

std::chrono::milliseconds UdcClient::ping() const
{
    return m_ping;
}

const UdcAddressMux& UdcClient::outgoingAddress() const
{
    return m_outgoingAddress;
}

const UdcAddressMux& UdcClient::incomingAddress() const
{
    return m_incomingAddress;
}

void UdcClient::startConnecting(std::chrono::milliseconds time)
{
    m_firstConnectAttemptTime = time;
    m_prevConnectAttemptTime = std::chrono::milliseconds(0);
}

void UdcClient::retryConnecting(std::chrono::milliseconds time)
{
    m_prevConnectAttemptTime = time;
}

void UdcClient::setConnectionLost()
{
    m_isConnected = false;
}

bool UdcClient::needsConnectionTimeoutEvent(std::chrono::milliseconds time) const
{
    return (time - m_firstConnectAttemptTime) >= m_connectionTimeoutPeriod;
}

bool UdcClient::needsConnectionAttempt(std::chrono::milliseconds time) const
{
    return (time - m_prevConnectAttemptTime) >= m_connectionAttemptPeriod;
}

bool UdcClient::receivePong(std::chrono::milliseconds pingSentTime, std::chrono::milliseconds pongReceivedTime)
{
    m_ping = pongReceivedTime - pingSentTime;
    m_pingLastSetTime = pongReceivedTime;
    m_lastReceivedTime = pongReceivedTime;

    if (!m_isConnected)
    {
        m_isConnected = true;
        return true;
    }

    return false;
}

bool UdcClient::receiveReliableHandshake(std::chrono::milliseconds reliableSentTime, std::chrono::milliseconds handshakeReceivedTime)
{
    // Allow reliable message handshake timestamp to act like ping/pong
    // there's no reason to send excessive pings if reliable traffic is high
    // and this can regain a connection as well
    return receivePong(reliableSentTime, handshakeReceivedTime);
}

void UdcClient::resetSendReliable()
{
    m_reliableSentTime = std::chrono::milliseconds(0);
}

void UdcClient::setSendReliable(std::chrono::milliseconds time)
{
    if (m_reliableSentTime == std::chrono::milliseconds(0))
    {
        m_reliableSentTime = time;
    }
}

bool UdcClient::needsReliableReset(std::chrono::milliseconds time) const
{
    if (m_reliableSentTime == std::chrono::milliseconds(0))
    {
        return false;
    }

    return (time - m_reliableSentTime >= m_reliableTimeoutPeriod);
}

bool UdcClient::needsPing(std::chrono::milliseconds time) const
{
    return (time - m_pingLastSetTime) >= m_pingPeriod;
}

bool UdcClient::needsConnectionLostEvent(std::chrono::milliseconds time) const
{
    return m_isConnected && ((time - m_lastReceivedTime) >= m_connectionLostPeriod);
}

void UdcClient::receiveConnectionHandshake(std::chrono::milliseconds receivedTime)
{
    if (!m_isConnected)
    {
        m_isConnected = true;
    }

    m_lastReceivedTime = receivedTime;
}

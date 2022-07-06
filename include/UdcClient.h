// udp-connect
// Kyle J Burgess

#ifndef UDC_CLIENT_H
#define UDC_CLIENT_H

#include "udp_connect.h"
#include "UdcTypes.h"
#include "UdcSocketMux.h"
#include "UdcMessage.h"

#include <cstdint>
#include <vector>
#include <chrono>
#include <queue>

class UdcClient
{
public:
    UdcClient(
        UdcEndPointId endPointId,
        const UdcAddressMux& outgoingAddress,
        std::chrono::milliseconds pingPeriod,
        std::chrono::milliseconds timeoutPeriod);

    // Get client id
    [[nodiscard]]
    UdcEndPointId id() const;

    // The current reliable message ID to send
    [[nodiscard]]
    UdcMessageId reliableMsgId() const;

    // Sets the current reliable message ID
    void setReliableMsgId(UdcMessageId id);

    // The reliable message queue for this client
    [[nodiscard]]
    std::queue<std::vector<uint8_t>>& reliableMessages();

    // Returns true if the client is connected
    [[nodiscard]]
    bool connected() const;

    // Get client ping
    [[nodiscard]]
    std::chrono::milliseconds ping() const;

    // Get client outgoing address
    [[nodiscard]]
    const UdcAddressMux& outgoingAddress() const;

    // Get client incoming address
    [[nodiscard]]
    const UdcAddressMux& incomingAddress() const;

    void startConnecting(std::chrono::milliseconds time);

    void retryConnecting(std::chrono::milliseconds time);

    void setConnectionLost();

    [[nodiscard]]
    bool needsConnectionTimeoutEvent(std::chrono::milliseconds time) const;

    [[nodiscard]]
    bool needsConnectionAttempt(std::chrono::milliseconds time) const;

    // Set client ping (ms) after receiving PONG message
    // refreshes last received timer
    // returns true if connection has been regained
    [[nodiscard]]
    bool receivePong(std::chrono::milliseconds pingSentTime, std::chrono::milliseconds pongReceivedTime);

    // Receive a handshake
    void receiveHandshake(std::chrono::milliseconds receivedTime);

    // Client needs a ping
    // It has been longer than pingPeriod since the last time
    // this client's ping was set.
    [[nodiscard]]
    bool needsPing(std::chrono::milliseconds time) const;

    // Client should get timed out
    // time since last received is longer than timeoutPeriod
    [[nodiscard]]
    bool needsConnectionLostEvent(std::chrono::milliseconds time) const;

protected:
    UdcEndPointId m_id;

    // The reliable message id
    UdcMessageId m_reliableMsgId;

    // The reliable message queue for this client
    std::queue<std::vector<uint8_t>> m_reliableMessages;

    // True when first connected,
    // false if UDC_EVENT_CONNECTION_LOST
    // and set back to true when UDC_EVENT_CONNECTION_REGAINED
    bool m_isConnected;

    UdcAddressMux m_outgoingAddress; // The address that this server sends to
    UdcAddressMux m_incomingAddress; // The address that this server gets back

    std::chrono::milliseconds m_pingPeriod; // how often ping is queried
    std::chrono::milliseconds m_connectionTimeoutPeriod; // how long before a connection times out
    std::chrono::milliseconds m_connectionLostPeriod; // how long before a connection times out
    std::chrono::milliseconds m_connectionAttemptPeriod; // how long between connection attempts

    std::chrono::milliseconds m_ping; // the last retrieved ping value
    std::chrono::milliseconds m_pingLastSetTime; // last time a ping was sent
    std::chrono::milliseconds m_lastReceivedTime; // time since last message received

    std::chrono::milliseconds m_firstConnectAttemptTime;
    std::chrono::milliseconds m_prevConnectAttemptTime;
};

#endif

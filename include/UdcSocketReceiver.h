// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_RECEIVER_H
#define UDC_SOCKET_RECEIVER_H

#include "udp_connect.h"

#include <cstdint>
#include <string>
#include <vector>
#include <winsock2.h>

// UdcSocketReceiver
// Manages receiving UDP packets on a local port
class UdcSocketReceiver
{
public:
    using Buffer = const uint8_t*;

    explicit UdcSocketReceiver(size_t maxMessageSize);

    UdcSocketReceiver(const UdcSocketReceiver&) = delete;

    UdcSocketReceiver& operator=(const UdcSocketReceiver&) = delete;

    ~UdcSocketReceiver();

    bool connect(uint16_t localPort);

    // Receive messages from the connected port and return true
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    bool receive(
        UdcAddressFamily& addressFamily,
        UdcAddressIPv4& addressIPv4,
        UdcAddressIPv6& addressIPv6,
        Buffer& message,
        size_t& messageSize);

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    bool m_wsaStarted;
    bool m_connected;
    SOCKET m_socket;
    std::vector<uint8_t> m_buffer;
};

#endif

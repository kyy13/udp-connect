// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_RECEIVER_H
#define UDC_SOCKET_RECEIVER_H

#include "UdcSocket.h"

union UdcAddress
{
    UdcAddressIPv4 ipv4;
    UdcAddressIPv6 ipv6;
};

// UdcSocketReceiver
// Manages receiving UDP packets on a local port
class UdcSocketReceiver
{
public:

    // Attempt to bind primaryPort as a dual-stack IPv6 port that's capable of receiving IPv4 and IPv6
    // If dual-stack is not available, then binds an IPv6 port on primaryPort and an IPv4 port on backupPort
    [[nodiscard]]
    bool bind(
        uint16_t primaryPortIPv6,
        uint16_t backupPortIPv4);

    // Receive messages from the connected port and returns true
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    bool receive(
        UdcAddressFamily& addressFamily,
        UdcAddress& address,
        std::vector<uint8_t>& data);

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    UdcSocket m_primarySocket;
    UdcSocket m_backupSocket;
};

#endif

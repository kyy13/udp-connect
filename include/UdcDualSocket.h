// udp-connect
// Kyle J Burgess

#ifndef UDC_DUAL_SOCKET_H
#define UDC_DUAL_SOCKET_H

#include "UdcSocket.h"

// UdcDualSocket
// Manages sending and receiving UDP packets on IPv4 and IPv6
class UdcDualSocket
{
public:

    // Attempt to bind primaryPort as a dual-stack IPv6 port that's capable of receiving IPv4 and IPv6
    // If dual-stack is not available, then binds an IPv6 port on primaryPort and an IPv4 port on backupPort
    [[nodiscard]]
    bool bind(uint16_t portIPv6, uint16_t portIPv4);

    // Send a message
    bool send(const UdcAddressIPv4& address, uint16_t port, const std::vector<uint8_t>& data) const;

    // Send a message
    bool send(const UdcAddressIPv6& address, uint16_t port, const std::vector<uint8_t>& data) const;

    // Receive messages from the connected port and
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    [[nodiscard]]
    bool receive(
        UdcAddressIPv4& address,
        uint16_t& port,
        std::vector<uint8_t>& data);

    // Receive messages from the connected port and
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    [[nodiscard]]
    bool receive(
        UdcAddressIPv6& address,
        uint16_t& port,
        std::vector<uint8_t>& data);

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    UdcSocket m_socketIPv4;
    UdcSocket m_socketIPv6;
};

#endif

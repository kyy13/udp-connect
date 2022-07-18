// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_MUX_H
#define UDC_SOCKET_MUX_H

#include "UdcSocket.h"
#include "UdcAddressMux.h"
#include "UdcPacketLogger.h"

#include <vector>
#include <memory>

// UdcSocketMux
// Manages sending and receiving UDP packets on IPv4 and IPv6
class UdcSocketMux
{
public:

    UdcSocketMux();

    UdcSocketMux(const std::string& logFileName);

    [[nodiscard]]
    bool tryBindIPv4(uint16_t port);

    [[nodiscard]]
    bool tryBindIPv6(uint16_t port);

    // Send a message
    bool send(const UdcAddressMux& address, const uint8_t* data, uint32_t size) const;

    // Send a message
    bool send(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size) const;

    // Send a message
    bool send(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size) const;

    // Receive messages from the connected port and
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    [[nodiscard]]
    bool receive(
        UdcAddressMux& address,
        uint8_t* buffer,
        uint32_t& size);

    // Receive messages from the connected port and
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    [[nodiscard]]
    bool receive(
        UdcAddressIPv4& address,
        uint16_t& port,
        uint8_t* buffer,
        uint32_t& size);

    // Receive messages from the connected port and
    // returns false when there are no messages to receive
    // ignores messages that are larger than maxMessageSize
    [[nodiscard]]
    bool receive(
        UdcAddressIPv6& address,
        uint16_t& port,
        uint8_t* buffer,
        uint32_t& size);

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    std::vector<UdcSocket> m_socketIPv4;
    std::vector<UdcSocket> m_socketIPv6;
    std::unique_ptr<UdcPacketLogger> m_logger;
};

#endif

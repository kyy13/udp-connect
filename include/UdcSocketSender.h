// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_SENDER_H
#define UDC_SOCKET_SENDER_H

#include "UdcSocket.h"

// UdcSocketSender
// Manages sending UDP packets to an ip/port
class UdcSocketSender
{
public:

    // Open and configure a socket, then establish a destination ip/port.
    // Connect can be called multiple times without calling disconnect()
    // in order to change the ip/port.
    [[nodiscard]]
    bool connect(const std::string& nodeName, const std::string& serviceName);

    [[nodiscard]]
    bool connectIPv4(const UdcAddressIPv4& remoteIp, uint16_t remotePort);

    [[nodiscard]]
    bool connectIPv6(const UdcAddressIPv6& remoteIp, uint16_t remotePort);

    // Send a message to connected port
    [[nodiscard]]
    bool send(const std::vector<uint8_t>& data) const;

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    UdcSocket m_socket;
};

#endif

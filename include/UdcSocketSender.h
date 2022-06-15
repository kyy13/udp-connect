// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_SENDER_H
#define UDC_SOCKET_SENDER_H

#include "udp_connect.h"
#include <winsock2.h>
#include <cstdint>

// UdcSocketSender
// Manages sending UDP packets to an ip/port
class UdcSocketSender
{
public:
    UdcSocketSender();
    UdcSocketSender(const UdcSocketSender&) = delete;
    UdcSocketSender& operator=(const UdcSocketSender&) = delete;
    ~UdcSocketSender();

    // Open and configure a socket, then establish a destination ip/port.
    // Connect can be called multiple times without calling disconnect()
    // in order to change the ip/port.
    bool connect(UdcAddressIPv4 remoteIp, uint16_t remotePort);
    bool connect(UdcAddressIPv6 remoteIp, uint16_t remotePort);

    // Send a message to connected port
    bool send(const uint8_t* buffer, size_t size) const;

    // Manually closes the socket
    void disconnect();

    // Checks connection status
    [[nodiscard]]
    bool isConnected() const;

protected:
    bool m_wsaStarted;
    bool m_connected;
    SOCKET m_socket;
};

#endif

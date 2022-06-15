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
    UdcSocketReceiver(size_t maxMessageSize);
    UdcSocketReceiver(const UdcSocketReceiver&) = delete;
    UdcSocketReceiver& operator=(const UdcSocketReceiver&) = delete;
    ~UdcSocketReceiver();

    bool connect(uint16_t port);

    // Receive messages from the connected port
    // ignoring messages that are larger than maxMessageSize
    bool receive(IpVersion& version, IpAddress& address, uint16_t& port, uint8_t* msg, size_t& size);

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

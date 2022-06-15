// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_SENDER_H
#define UDC_SOCKET_SENDER_H

#include <string>
#include <winsock2.h>

class UdcSocketSender
{
public:
    UdcSocketSender();
    UdcSocketSender(const UdcSocketSender&) = delete;
    UdcSocketSender& operator=(const UdcSocketSender&) = delete;
    ~UdcSocketSender();

    // Connect to a port for sending
    // calling connect again will disconnect,
    // and then reconnect to the specified port
    bool connect(const char* ip, u_short port);

    // Send a message to connected port
    bool send(const char* msg) const;

    // Manually disconnect from a port
    void disconnect();

protected:
    bool m_wsaStarted;
    bool m_connected;
    SOCKET m_socket;
};

#endif

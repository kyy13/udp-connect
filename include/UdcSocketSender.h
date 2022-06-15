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

    bool connect(const char* ip, u_short port);

    bool send(const char* msg) const;

protected:
    bool m_wsaStarted;
    bool m_connected;
    SOCKET m_socket;
};

#endif

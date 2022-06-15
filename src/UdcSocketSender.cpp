// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcCommon.h"

#include <cstring>
#include <Ws2tcpip.h>

UdcSocketSender::UdcSocketSender()
    : m_wsaStarted(false)
    , m_connected(false)
    , m_socket(INVALID_SOCKET)
{}

UdcSocketSender::~UdcSocketSender()
{
    if (m_wsaStarted)
    {
        WSACleanup();
    }
}

bool UdcSocketSender::connect(const char* ip, u_short port)
{
    m_connected = false;

    // Start WSA if necessary
    if (!m_wsaStarted)
    {
        WSADATA wsaData;

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
        {
            return false;
        }

        m_wsaStarted = true;
    }

    // Close previous socket if necessary
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    // Create a socket
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    // Connect to ip/port

    in_addr ip_addr {};
    ip_addr.s_addr = inet_addr(ip);

    sockaddr_in address =
        {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = ip_addr,
            .sin_zero = {0,0,0,0,0,0,0,0},
        };

    if (!setSocketNonBlocking(m_socket))
    {
        return false;
    }

    if (::connect(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        return false;
    }

    m_connected = true;
    return true;
}

bool UdcSocketSender::send(const char* msg) const
{
    if (!m_connected)
    {
        return false;
    }

    return ::send(m_socket, msg, static_cast<int>(strlen(msg)), 0) != SOCKET_ERROR;
}

// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcCommon.h"

#include <cstring>
#include <ws2tcpip.h>

UdcSocketSender::UdcSocketSender()
    : m_wsaStarted(false)
    , m_connected(false)
    , m_socket(INVALID_SOCKET)
{}

UdcSocketSender::~UdcSocketSender()
{
    // Close previous socket if necessary
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    // Cleanup WSA lib
    if (m_wsaStarted)
    {
        WSACleanup();
    }
}

bool UdcSocketSender::connect(const char* ip, u_short localPort, u_short remotePort)
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

    // Establish socket info
    in_addr ip_addr {};
    ip_addr.s_addr = INADDR_ANY;

    sockaddr_in localAddress =
        {
            .sin_family = AF_INET,
            .sin_port = htons(localPort),
            .sin_addr = ip_addr,
            .sin_zero = {0,0,0,0,0,0,0,0},
        };

    ip_addr.s_addr = inet_addr(ip);

    sockaddr_in remoteAddress =
        {
            .sin_family = AF_INET,
            .sin_port = htons(remotePort),
            .sin_addr = ip_addr,
            .sin_zero = {0,0,0,0,0,0,0,0},
        };

    // Set socket non-blocking
    if (!setSocketNonBlocking(m_socket))
    {
        return false;
    }

    // Bind to sending port
    if (::bind(m_socket, reinterpret_cast<sockaddr*>(&localAddress), sizeof(localAddress)) == SOCKET_ERROR)
    {
        return false;
    }

    // Connect to remote port
    if (::connect(m_socket, reinterpret_cast<sockaddr*>(&remoteAddress), sizeof(remoteAddress)) == SOCKET_ERROR)
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

void UdcSocketSender::disconnect()
{
    // Close previous socket if necessary
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_connected = false;
}

bool UdcSocketSender::isConnected() const
{
    return m_connected;
}

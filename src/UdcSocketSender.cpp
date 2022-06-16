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

bool UdcSocketSender::connect(UdcAddressIPv4 remoteIp, uint16_t remotePort)
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

    // Set socket non-blocking
    if (!setSocketNonBlocking(m_socket))
    {
        return false;
    }

    in_addr ip_addr {};

    // Configure remote address
    ip_addr = convertFromIPv4(remoteIp);

    sockaddr_in remoteAddress;
    memset(&remoteAddress, 0, sizeof(remoteAddress));

    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_port = htons(remotePort);
    remoteAddress.sin_addr = ip_addr;

    // Connect to remote port
    if (::connect(m_socket, reinterpret_cast<sockaddr*>(&remoteAddress), sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        return false;
    }

    m_connected = true;
    return true;
}

bool UdcSocketSender::connect(UdcAddressIPv6 remoteIp, uint16_t remotePort)
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
    m_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    // Set socket non-blocking
    if (!setSocketNonBlocking(m_socket))
    {
        return false;
    }

    // Configure remote address
    sockaddr_in6 remoteAddress;
    memset(&remoteAddress, 0, sizeof(remoteAddress));

    remoteAddress.sin6_family = AF_INET6;
    remoteAddress.sin6_port = htons(remotePort);
    remoteAddress.sin6_addr = convertFromIPv6(remoteIp);

    // Connect to remote port
    if (::connect(m_socket, reinterpret_cast<sockaddr*>(&remoteAddress), sizeof(sockaddr_in6)) == SOCKET_ERROR)
    {
        return false;
    }

    m_connected = true;
    return true;
}

bool UdcSocketSender::send(const uint8_t* buffer, size_t size) const
{
    if (!m_connected)
    {
        return false;
    }

    return ::send(m_socket, reinterpret_cast<const char*>(buffer), static_cast<int>(size), 0) != SOCKET_ERROR;
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

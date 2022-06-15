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

bool UdcSocketSender::connect(IpVersion version, IpAddress address, uint16_t localPort, uint16_t remotePort)
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

    // Establish socket info
    union
    {
        sockaddr_in sa4;
        sockaddr_in6 sa6;
    } localAddress, remoteAddress;

    sockaddr* pLocalAddress;
    sockaddr* pRemoteAddress;
    int nameLen;

    if (version == IP_V6) // IPv6
    {
        in6_addr ip_addr {};

        ip_addr = IN6ADDR_ANY_INIT;
        localAddress.sa6 =
            {
                .sin6_family = AF_INET6,
                .sin6_port = htons(localPort),
                .sin6_addr = ip_addr,
            };
        pLocalAddress = reinterpret_cast<sockaddr*>(&localAddress.sa6);

        ip_addr = convertIPv6(address);
        remoteAddress.sa6 =
            {
                .sin6_family = AF_INET6,
                .sin6_port = htons(remotePort),
                .sin6_addr = ip_addr,
            };
        pRemoteAddress = reinterpret_cast<sockaddr*>(&remoteAddress.sa6);

        nameLen = sizeof(localAddress.sa6);
    }
    else // IPv4
    {
        in_addr ip_addr {};

        ip_addr.s_addr = INADDR_ANY;
        localAddress.sa4 =
            {
                .sin_family = AF_INET,
                .sin_port = htons(localPort),
                .sin_addr = ip_addr,
                .sin_zero = {0,0,0,0,0,0,0,0},
            };
        pLocalAddress = reinterpret_cast<sockaddr*>(&localAddress.sa4);

        ip_addr = convertIPv4(address);
        remoteAddress.sa4 =
            {
                .sin_family = AF_INET,
                .sin_port = htons(remotePort),
                .sin_addr = ip_addr,
                .sin_zero = {0,0,0,0,0,0,0,0},
            };
        pRemoteAddress = reinterpret_cast<sockaddr*>(&remoteAddress.sa4);

        nameLen = sizeof(localAddress.sa4);
    }

    // Bind to sending port
    if (::bind(m_socket, pLocalAddress, nameLen) == SOCKET_ERROR)
    {
        return false;
    }

    // Connect to remote port
    if (::connect(m_socket, pRemoteAddress, nameLen) == SOCKET_ERROR)
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

// udp-connect
// Kyle J Burgess

#include "UdcSocket.h"
#include "UdcSocketHelper.h"

#include <ws2tcpip.h>

#ifdef OS_WINDOWS
#include <iostream>
std::vector<uint8_t> UdcSocket::m_buffer;

UdcSocket::UdcSocket()
    : m_socket(INVALID_SOCKET)
{}

UdcSocket::~UdcSocket()
{
    deleteSocket(m_socket);
}

bool UdcSocket::isConnected() const
{
    return m_socket != INVALID_SOCKET;
}

bool UdcSocket::remoteConnect(const UdcAddressIPv4& remoteIp, uint16_t remotePort)
{
    SOCKET s = createSocket(AF_INET);

    if (s == INVALID_SOCKET)
    {
        std::cout << "1";
        return false;
    }

    if (!setSocketOptionNonBlocking(s, true))
    {
        std::cout << "2";
        deleteSocket(s);
        return false;
    }

    if (!connectSocket(s, createAddressIPv4(remoteIp, remotePort)))
    {
        std::cout << "3";
        deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

bool UdcSocket::remoteConnect(const UdcAddressIPv6& remoteIp, uint16_t remotePort)
{
    SOCKET s = createSocket(AF_INET6);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    if (!setSocketOptionNonBlocking(s, true))
    {
        deleteSocket(s);
        return false;
    }

    if (!connectSocket(s, createAddressIPv6(remoteIp, remotePort)))
    {
        deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

bool UdcSocket::localBindIPv4(uint16_t localPort)
{
    SOCKET s = createSocket(AF_INET);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    if (!setSocketOptionNonBlocking(s, true))
    {
        deleteSocket(s);
        return false;
    }

    if (!bindSocket(s, createAddressIPv4(INADDR_ANY, localPort)))
    {
        deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

bool UdcSocket::localBindIPv6(uint16_t localPort, bool allowIPv4)
{
    SOCKET s = createSocket(AF_INET6);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    if (!setSocketOptionNonBlocking(s, true))
    {
        deleteSocket(s);
        return false;
    }

    if (allowIPv4)
    {
        if (!setSocketOptionIpv6Only(s, false))
        {
            deleteSocket(s);
            return false;
        }
    }

    if (!bindSocket(s, createAddressIPv6(IN6ADDR_ANY_INIT, localPort)))
    {
        deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

void UdcSocket::disconnect()
{
    deleteSocket(m_socket);
}

bool UdcSocket::send(const std::vector<uint8_t>& data) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    return sendPacket(m_socket, data);
}

int32_t UdcSocket::receive(UdcAddressIPv4& sourceIP, std::vector<uint8_t>& data, size_t maxSize) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    m_buffer.resize(maxSize);
    return receivePacketIPv4(m_socket, m_buffer, sourceIP, data);
}

int32_t UdcSocket::receive(UdcAddressIPv6& sourceIP, std::vector<uint8_t>& data, size_t maxSize) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    m_buffer.resize(maxSize);
    return receivePacketIPv6(m_socket, m_buffer, sourceIP, data);
}

#endif

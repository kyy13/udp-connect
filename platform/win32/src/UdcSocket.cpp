// udp-connect
// Kyle J Burgess

#include "UdcSocket.h"
#include "UdcSocketHelper.h"

#include <ws2tcpip.h>
#include <stdexcept>

UdcSocket::UdcSocket()
    : m_socket(INVALID_SOCKET)
{
    if (!WinSock::startWSA())
    {
        throw std::runtime_error("Could not start Winsock DLL!");
    }
}

UdcSocket::~UdcSocket()
{
    WinSock::deleteSocket(m_socket);
    WinSock::stopWSA();
}

UdcSocket::UdcSocket(UdcSocket&& o) noexcept
    : m_socket(o.m_socket)
{
    if (!WinSock::startWSA())
    {
        throw std::runtime_error("Could not start Winsock DLL!");
    }

    o.m_socket = INVALID_SOCKET;
}

UdcSocket& UdcSocket::operator=(UdcSocket&& o) noexcept
{
    if (this != &o)
    {
        WinSock::deleteSocket(m_socket);
        m_socket = o.m_socket;
        o.m_socket = INVALID_SOCKET;
    }

    return *this;
}

bool UdcSocket::isConnected() const
{
    return m_socket != INVALID_SOCKET;
}

bool UdcSocket::stringToIPv6(
    const std::string& nodeName,
    const std::string& serviceName,
    UdcAddressIPv6& dstAddress,
    uint16_t& dstPort)
{
    if (!WinSock::startWSA())
    {
        return false;
    }

    // Setup address hints
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Setup linked-list of address results
    addrinfo* addressList = nullptr;

    // Get address info
    if (getaddrinfo(nodeName.c_str(), serviceName.c_str(), &hints, &addressList) != 0)
    {
        WinSock::stopWSA();
        return false;
    }

    // Iterate over result linked list
    for(addrinfo* address = addressList; address != nullptr; address = address->ai_next)
    {
        if (address->ai_socktype != SOCK_DGRAM || address->ai_protocol != IPPROTO_UDP)
        {
            continue;
        }

        if (address->ai_family == AF_INET6)
        {
            auto* sa = reinterpret_cast<sockaddr_in6*>(address->ai_addr);

            dstPort = ntohs(sa->sin6_port);
            WinSock::convertInaddrToIPv6(sa->sin6_addr, dstAddress);

            WinSock::stopWSA();
            return true;
        }
    }

    WinSock::stopWSA();
    return false;
}

bool UdcSocket::stringToIPv4(
    const std::string& nodeName,
    const std::string& serviceName,
    UdcAddressIPv4& dstAddress,
    uint16_t& dstPort)
{
    if (!WinSock::startWSA())
    {
        return false;
    }

    // Setup address hints
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Setup linked-list of address results
    addrinfo* addressList = nullptr;

    // Get address info
    int result = getaddrinfo(nodeName.c_str(), serviceName.c_str(), &hints, &addressList);
    if (result != 0)
    {
        WinSock::stopWSA();
        return false;
    }

    // Iterate over result linked list
    for(addrinfo* address = addressList; address != nullptr; address = address->ai_next)
    {
        if (address->ai_socktype != SOCK_DGRAM || address->ai_protocol != IPPROTO_UDP)
        {
            continue;
        }

        if (address->ai_family == AF_INET)
        {
            auto* sa = reinterpret_cast<sockaddr_in*>(address->ai_addr);

            dstPort = ntohs(sa->sin_port);
            WinSock::convertInaddrToIPv4(sa->sin_addr, dstAddress);

            WinSock::stopWSA();
            return true;
        }
    }

    WinSock::stopWSA();
    return false;
}

bool UdcSocket::remoteConnectIPv4()
{
    SOCKET s = WinSock::createSocket(AF_INET);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    m_socket = s;

    return true;
}

bool UdcSocket::remoteConnectIPv6()
{
    SOCKET s = WinSock::createSocket(AF_INET6);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    m_socket = s;

    return true;
}

bool UdcSocket::localBindIPv4(uint16_t localPort)
{
    SOCKET s = WinSock::createSocket(AF_INET);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    if (!WinSock::setSocketOptionNonBlocking(s, true))
    {
        WinSock::deleteSocket(s);
        return false;
    }

    if (!WinSock::bindSocketIPv4(s, WinSock::createAddressIPv4(INADDR_ANY, localPort)))
    {
        WinSock::deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

bool UdcSocket::localBindIPv6(uint16_t localPort, bool allowIPv4)
{
    SOCKET s = WinSock::createSocket(AF_INET6);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    if (!WinSock::setSocketOptionNonBlocking(s, true))
    {
        WinSock::deleteSocket(s);
        return false;
    }

    if (allowIPv4)
    {
        if (!WinSock::setSocketOptionIpv6Only(s, false))
        {
            WinSock::deleteSocket(s);
            return false;
        }
    }

    if (!WinSock::bindSocketIPv6(s, WinSock::createAddressIPv6(IN6ADDR_ANY_INIT, localPort)))
    {
        WinSock::deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

void UdcSocket::disconnect()
{
    WinSock::deleteSocket(m_socket);
}

bool UdcSocket::sendIPv4(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    return WinSock::sendPacketIPv4(m_socket, WinSock::createAddressIPv4(address, port), data, size);
}

bool UdcSocket::sendIPv6(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    return WinSock::sendPacketIPv6(m_socket, WinSock::createAddressIPv6(address, port), data, size);
}

int32_t UdcSocket::receiveIPv4(UdcAddressIPv4& sourceIP, uint16_t& port, uint8_t* buffer, uint32_t& size) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    return WinSock::receivePacketIPv4(m_socket, sourceIP, port, buffer, size);
}

int32_t UdcSocket::receiveIPv6(UdcAddressIPv6& sourceIP, uint16_t& port, uint8_t* buffer, uint32_t& size) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    return WinSock::receivePacketIPv6(m_socket, sourceIP, port, buffer, size);
}

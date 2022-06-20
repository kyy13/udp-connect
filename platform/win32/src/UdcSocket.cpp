// udp-connect
// Kyle J Burgess

#include "UdcSocket.h"
#include "UdcSocketHelper.h"

#include <ws2tcpip.h>
#include <stdexcept>

std::vector<uint8_t> UdcSocket::m_buffer;

UdcSocket::UdcSocket()
    : m_socket(INVALID_SOCKET)
{
    if (!startWSA())
    {
        throw std::runtime_error("Could not start Winsock DLL!");
    }
}

UdcSocket::~UdcSocket()
{
    deleteSocket(m_socket);
    stopWSA();
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
            convertInaddrToIPv6(sa->sin6_addr, dstAddress);

            return true;
        }
    }

    return false;
}

bool UdcSocket::stringToIPv4(
    const std::string& nodeName,
    const std::string& serviceName,
    UdcAddressIPv4& dstAddress,
    uint16_t& dstPort)
{
    // Setup address hints
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    // Setup linked-list of address results
    addrinfo* addressList = nullptr;

    // Get address info
    if (getaddrinfo(nodeName.c_str(), serviceName.c_str(), &hints, &addressList) != 0)
    {
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
            convertInaddrToIPv4(sa->sin_addr, dstAddress);

            return true;
        }
    }

    return false;
}

bool UdcSocket::remoteConnect(const std::string& nodeName, const std::string& serviceName)
{
    UdcAddressIPv6 addressIPv6;
    uint16_t port;

    if (stringToIPv6(nodeName, serviceName, addressIPv6, port) &&
        remoteConnectIPv6(addressIPv6, port))
    {
        return true;
    }

    UdcAddressIPv4 addressIPv4;
    if (stringToIPv4(nodeName, serviceName, addressIPv4, port) &&
        remoteConnectIPv4(addressIPv4, port))
    {
        return true;
    }

    return false;
}

bool UdcSocket::remoteConnectIPv4(const UdcAddressIPv4& remoteIp, uint16_t remotePort)
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

    if (!connectSocket(s, createAddressIPv4(remoteIp, remotePort)))
    {
        deleteSocket(s);
        return false;
    }

    m_socket = s;
    return true;
}

bool UdcSocket::remoteConnectIPv6(const UdcAddressIPv6& remoteIp, uint16_t remotePort)
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

int32_t UdcSocket::receiveIPv4(UdcAddressIPv4& sourceIP, std::vector<uint8_t>& data, size_t maxSize) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    m_buffer.resize(maxSize);
    return receivePacketIPv4(m_socket, m_buffer, sourceIP, data);
}

int32_t UdcSocket::receiveIPv6(UdcAddressIPv6& sourceIP, std::vector<uint8_t>& data, size_t maxSize) const
{
    if (m_socket == INVALID_SOCKET)
    {
        return -2;
    }

    m_buffer.resize(maxSize);
    return receivePacketIPv6(m_socket, m_buffer, sourceIP, data);
}

// udp-connect
// Kyle J Burgess

#include "UdcSocketHelper.h"

#include <cassert>

SOCKET createSocket(int protocol)
{
    WSAData wsaData;

    // Start WSA v2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return INVALID_SOCKET;
    }

    // Create socket
    SOCKET s = socket(protocol, SOCK_DGRAM, IPPROTO_UDP);

    // If WSA was started and socket creation failed, then cleanup WSA
    if (s == INVALID_SOCKET)
    {
        WSACleanup();
    }

    return s;
}

void deleteSocket(SOCKET& s)
{
    if (s != INVALID_SOCKET)
    {
        closesocket(s);
        s = INVALID_SOCKET;
        WSACleanup();
    }
}

bool bindSocket(SOCKET s, sockaddr_in address)
{
    return bind(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
}

bool bindSocket(SOCKET s, sockaddr_in6 address)
{
    return bind(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
}

bool connectSocket(SOCKET s, sockaddr_in address)
{
    return connect(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
}

bool connectSocket(SOCKET s, sockaddr_in6 address)
{
    return connect(s, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
}

bool setSocketOptionNonBlocking(SOCKET socket, bool noBlock)
{
    u_long mode = noBlock ? 1 : 0;
    return ioctlsocket(socket, FIONBIO, &mode) != SOCKET_ERROR;
}

bool setSocketOptionIpv6Only(SOCKET socket, bool ipv6Only)
{
    u_long opt = ipv6Only ? 1 : 0;
    return setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&opt), sizeof(opt)) != SOCKET_ERROR;
}

sockaddr_in createAddressIPv4(const UdcAddressIPv4& address, uint16_t port)
{
    sockaddr_in result;
    memset(&result, 0, sizeof(result));
    memcpy(&result.sin_addr.S_un.S_un_b.s_b1, address.octets, sizeof(address.octets));
    result.sin_port = htons(port);
    return result;
}

sockaddr_in createAddressIPv4(u_long address, uint16_t port)
{
    sockaddr_in result;
    memset(&result, 0, sizeof(result));
    result.sin_addr.S_un.S_addr = address;
    result.sin_port = htons(port);
    return result;
}

void convertToIPv4(const in_addr& src, UdcAddressIPv4& dst)
{
    memcpy(dst.octets, &src.S_un.S_un_b.s_b1, sizeof(dst.octets));
}

sockaddr_in6 createAddressIPv6(const UdcAddressIPv6& address, uint16_t port)
{
    sockaddr_in6 result;
    memset(&result, 0, sizeof(result));
    memcpy(result.sin6_addr.u.Byte, address.segments, sizeof(address.segments));
    result.sin6_port = htons(port);
    return result;
}

sockaddr_in6 createAddressIPv6(const in6_addr& address, uint16_t port)
{
    sockaddr_in6 result;
    memset(&result, 0, sizeof(result));
    result.sin6_addr = address;
    result.sin6_port = htons(port);
    return result;
}

void convertToIPv6(const in6_addr& src, UdcAddressIPv6& dst)
{
    memcpy(dst.segments, src.u.Byte, sizeof(dst.segments));
}

bool sendPacket(SOCKET s, const std::vector<uint8_t>& data)
{
    return send(s, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0) != SOCKET_ERROR;
}

int32_t receivePacketIPv4(SOCKET s, std::vector<uint8_t>& tmpBuffer, UdcAddressIPv4& sourceIP, std::vector<uint8_t>& data)
{
    sockaddr_in ip;
    int ipSize = sizeof(ip);

    int result = recvfrom(s,
        reinterpret_cast<char*>(tmpBuffer.data()),
        static_cast<int>(tmpBuffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&ip),
        &ipSize);

    // Connection was closed
    if (result == 0)
    {
        return -2;
    }

    // Socket error
    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        // Message was too long
        if (error == WSAEMSGSIZE)
        {
            convertToIPv4(ip.sin_addr, sourceIP);

            data.resize(tmpBuffer.size());
            memcpy(data.data(), tmpBuffer.data(), tmpBuffer.size());

            return -1;
        }

        // No messages left
        if (error == WSAEWOULDBLOCK)
        {
            return 0;
        }

        // Fatal error
        return -2;
    }

    convertToIPv4(ip.sin_addr, sourceIP);

    assert(result <= tmpBuffer.size());
    data.resize(result);
    memcpy(data.data(), tmpBuffer.data(), result);

    return 1;
}

int32_t receivePacketIPv6(SOCKET s, std::vector<uint8_t>& tmpBuffer, UdcAddressIPv6& sourceIP, std::vector<uint8_t>& data)
{
    sockaddr_in6 ip;
    int ipSize = sizeof(ip);

    int result = recvfrom(s,
        reinterpret_cast<char*>(tmpBuffer.data()),
        static_cast<int>(tmpBuffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&ip),
        &ipSize);

    // Connection was closed
    if (result == 0)
    {
        return -2;
    }

    // Socket error
    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        // Message was too long
        if (error == WSAEMSGSIZE)
        {
            convertToIPv6(ip.sin6_addr, sourceIP);

            data.resize(tmpBuffer.size());
            memcpy(data.data(), tmpBuffer.data(), tmpBuffer.size());

            return -1;
        }

        // No messages left
        if (error == WSAEWOULDBLOCK)
        {
            return 0;
        }

        // Fatal error
        return -2;
    }

    convertToIPv6(ip.sin6_addr, sourceIP);

    assert(result <= tmpBuffer.size());
    data.resize(result);
    memcpy(data.data(), tmpBuffer.data(), result);

    return 1;
}

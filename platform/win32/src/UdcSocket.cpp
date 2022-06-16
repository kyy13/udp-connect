// udp-connect
// Kyle J Burgess

#include "UdcSocket.h"

#ifdef WINDOWS_OS

UdcSocket::UdcSocket()
    : m_socket(INVALID_SOCKET)
{}

// Creates a socket
// returns INVALID_SOCKET on failure
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

// Deletes a socket
void deleteSocket(SOCKET s)
{
    if (s != INVALID_SOCKET)
    {
        closesocket(s);
        WSACleanup();
    }
}

// Set socket non-blocking option (true = not-blocking)
bool setSocketNonBlocking(SOCKET socket)
{
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) != SOCKET_ERROR;
}

bool setSocketIpv6OnlyOff(SOCKET socket)
{
    u_long opt = 0;
    return setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&opt), sizeof(opt)) != SOCKET_ERROR;
}

bool UdcSocket::remoteConnect(UdcAddressIPv4 remoteIp, uint16_t remotePort)
{
    SOCKET s = createSocket(AF_INET);

    if (s == INVALID_SOCKET)
    {
        return false;
    }

    return true;
}

#endif

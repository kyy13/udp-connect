// udp-connect
// Kyle J Burgess

#include "UdcSocketReceiver.h"
#include "UdcCommon.h"

#include <cassert>
#include <iostream>

UdcSocketReceiver::UdcSocketReceiver(size_t maxMessageSize)
    : m_wsaStarted(false)
    , m_connected(false)
    , m_socket(INVALID_SOCKET)
{
    m_buffer.resize(maxMessageSize);
}

UdcSocketReceiver::~UdcSocketReceiver()
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

bool UdcSocketReceiver::connect(uint16_t localPort)
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

    // Try to allow socket to receive IPv4 packets as well
    // not available on Windows XP
    if (!setSocketIpv6OnlyOff(m_socket))
    {
        return false;
    }

    // Establish socket info
    sockaddr_in6 address;
    memset(&address, 0, sizeof(address));

    address.sin6_family = AF_INET6;
    address.sin6_port = htons(localPort);
    address.sin6_addr = IN6ADDR_ANY_INIT;

    // Bind to ip/port
    if (::bind(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        return false;
    }

    m_connected = true;
    return true;
}

bool UdcSocketReceiver::receive(
    UdcAddressFamily& addressFamily,
    UdcAddressIPv4& addressIPv4,
    UdcAddressIPv6& addressIPv6,
    Buffer& message,
    size_t& messageSize)
{
    if (!m_connected)
    {
        return false;
    }

    sockaddr_storage remoteAddress {};
    int remoteAddressSize = sizeof(sockaddr_storage);

    // Receive messages, ignoring messages with errors
    while(true)
    {
        int result = recvfrom(
            m_socket,
            reinterpret_cast<char*>(m_buffer.data()),
            static_cast<int>(m_buffer.size()),
            0,
            reinterpret_cast<sockaddr*>(&remoteAddress),
            &remoteAddressSize);

        // Connection was closed
        if (result == 0)
        {
            m_connected = false;
            return false;
        }

        // Socket error
        if (result == SOCKET_ERROR)
        {
            int error = WSAGetLastError();

            // Message was too long, skip
            if (error == WSAEMSGSIZE)
            {
                continue;
            }

            // No messages left
            if (error == WSAEWOULDBLOCK)
            {
                return false;
            }

            // Other error
            return false;
        }

        // No socket error, return the message

        message = m_buffer.data();
        messageSize = result;

        // Copy from buffer to msg
        // message size should be <= buffer size, guaranteed by call to recvfrom
        assert(messageSize <= m_buffer.size());

        // Return the ip/port
        if (remoteAddress.ss_family == AF_INET6)
        {
            // IPv6
            auto* address = reinterpret_cast<const sockaddr_in6*>(&remoteAddress);
            convertToIPv6(address->sin6_addr, addressIPv6);
            addressFamily = UDC_IPV6;
        }
        else
        {
            // IPv4
            auto* address = reinterpret_cast<const sockaddr_in*>(&remoteAddress);
            convertToIPv4(address->sin_addr, addressIPv4);
            addressFamily = UDC_IPV4;
        }

        return true;
    }
}

void UdcSocketReceiver::disconnect()
{
    // Close previous socket if necessary
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_connected = false;
}

bool UdcSocketReceiver::isConnected() const
{
    return m_connected;
}

// udp-connect
// Kyle J Burgess

#include "UdcSocketReceiver.h"
#include "UdcCommon.h"

#include <cassert>

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

bool UdcSocketReceiver::connect(uint16_t port)
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
    ip_addr.s_addr = inet_addr("127.0.0.1");

    sockaddr_in address =
        {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = ip_addr,
            .sin_zero = {0,0,0,0,0,0,0,0},
        };

    // Set socket non-blocking
    if (!setSocketNonBlocking(m_socket))
    {
        return false;
    }

    // Bind to ip/port
    if (bind(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR)
    {
        return false;
    }

    m_connected = true;
    return true;
}

bool UdcSocketReceiver::receive(IpVersion& version, IpAddress& address, uint16_t& port, uint8_t* msg, size_t& size)
{
    if (!m_connected)
    {
        return false;
    }

    sockaddr_in address {};
    int addressSize = sizeof(address);

    // Receive messages, ignoring messages with errors
    while(true)
    {
        int result = recvfrom(
            m_socket,
            reinterpret_cast<char*>(m_buffer.data()),
            static_cast<int>(m_buffer.size()),
            0,
            reinterpret_cast<sockaddr*>(&address),
            &addressSize);

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
        msg.resize(result);

        // Copy from buffer to msg
        // message size should be <= buffer size, guaranteed by call to recvfrom
        assert(msg.size() <= m_buffer.size());
        memcpy(msg.data(), m_buffer.data(), msg.size());

        // Return the ip/port
        port = ntohs(address.sin_port);
        convertToIp(&address, ip);

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

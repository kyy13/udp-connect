// udp-connect
// Kyle J Burgess

#ifndef UDC_WIN32_SOCKET_HELPER_H
#define UDC_WIN32_SOCKET_HELPER_H

#include "udp_connect.h"

#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace WinSock
{
    // Starts winsock API
    // reference counted, so it will only be started when startWSA() call count = 0
    [[nodiscard]]
    bool startWSA();

    // Stop winsock API
    // reference counted, so it will be stopped when startWSA() call count == stopWSA() call count
    void stopWSA();

    // Creates a socket
    // returns INVALID_SOCKET on failure
    [[nodiscard]]
    SOCKET createSocket(int protocol);

    // Deletes a socket
    void deleteSocket(SOCKET& s);

    // Binds a socket to an IPv4 address
    [[nodiscard]]
    bool bindSocketIPv4(SOCKET s, sockaddr_in address);

    // Binds a socket to an IPv6 address
    [[nodiscard]]
    bool bindSocketIPv6(SOCKET s, sockaddr_in6 address);

    // Set socket non-blocking option (true = not-blocking)
    [[nodiscard]]
    bool setSocketOptionNonBlocking(SOCKET socket, bool noBlock);

    // Set an IPv6 socket to receive IPv6 only
    [[nodiscard]]
    bool setSocketOptionIpv6Only(SOCKET socket, bool ipv6Only);

    // Create a sockaddr_in struct from an IPv4 address and port
    [[nodiscard]]
    sockaddr_in createAddressIPv4(const UdcAddressIPv4& address, uint16_t port);

    // Create a sockaddr_in struct from an IPv4 address and port
    [[nodiscard]]
    sockaddr_in createAddressIPv4(u_long address, uint16_t port);

    // Create a sockaddr_in6 struct from an IPv6 address and port
    [[nodiscard]]
    sockaddr_in6 createAddressIPv6(const UdcAddressIPv6& address, uint16_t port);

    // Create a sockaddr_in6 struct from an IPv6 address and port
    [[nodiscard]]
    sockaddr_in6 createAddressIPv6(const in6_addr& address, uint16_t port);

    // Convert an in_addr into an IPv4
    void convertInaddrToIPv4(const in_addr& src, UdcAddressIPv4& dst);

    // Convert an in6_addr into an IPv6
    void convertInaddrToIPv6(const in6_addr& src, UdcAddressIPv6& dst);

    // Send a packet over IPv4
    // returns true on success
    [[nodiscard]]
    bool sendPacketIPv4(SOCKET s, sockaddr_in address, const uint8_t* data, uint32_t size);

    // Send a packet over IPv6
    // returns true on success
    [[nodiscard]]
    bool sendPacketIPv6(SOCKET s, sockaddr_in6 address, const uint8_t* data, uint32_t size);

    // Receive a packet on an IPv4 port
    // tmpBuffer is a buffer for holding temporary packet memory with size = max size of received packet
    // returns 1 on success
    // returns 0 if there are no messages left to receive
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receivePacketIPv4(SOCKET s, UdcAddressIPv4& sourceIP, uint16_t& sourcePort, uint8_t* buffer, uint32_t& size);

    // Receive a packet on an IPv6 port
    // tmpBuffer is a buffer for holding temporary packet memory with size = max size of received packet
    // returns 1 on success
    // returns 0 if there are no messages left to receive
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receivePacketIPv6(SOCKET s, UdcAddressIPv6& sourceIP, uint16_t& sourcePort, uint8_t* buffer, uint32_t& size);
}

#endif

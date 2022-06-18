// udp-connect
// Kyle J Burgess

#ifndef UDC_WIN32_SOCKET_HELPER_H
#define UDC_WIN32_SOCKET_HELPER_H

#include "udp_connect.h"

#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Starts winsock API
// reference counted, so it will only be started when startWSA() call count = 0
bool startWSA();

// Stop winsock API
// reference counted, so it will be stopped when startWSA() call count == stopWSA() call count
void stopWSA();

// Creates a socket
// returns INVALID_SOCKET on failure
SOCKET createSocket(int protocol);

// Deletes a socket
void deleteSocket(SOCKET& s);

// Binds a socket to an IPv4 address
bool bindSocket(SOCKET s, sockaddr_in address);

// Binds a socket to an IPv6 address
bool bindSocket(SOCKET s, sockaddr_in6 address);

// Connects a socket to an IPv4 address
bool connectSocket(SOCKET s, sockaddr_in address);

// Connects a socket to an IPv6 address
bool connectSocket(SOCKET s, sockaddr_in6 address);

// Set socket non-blocking option (true = not-blocking)
bool setSocketOptionNonBlocking(SOCKET socket, bool noBlock);

// Set an IPv6 socket to receive IPv6 only
bool setSocketOptionIpv6Only(SOCKET socket, bool ipv6Only);

// Create a sockaddr_in struct from an IPv4 address and port
sockaddr_in createAddressIPv4(const UdcAddressIPv4& address, uint16_t port);

// Create a sockaddr_in struct from an IPv4 address and port
sockaddr_in createAddressIPv4(u_long address, uint16_t port);

// Create a sockaddr_in6 struct from an IPv6 address and port
sockaddr_in6 createAddressIPv6(const UdcAddressIPv6& address, uint16_t port);

// Create a sockaddr_in6 struct from an IPv6 address and port
sockaddr_in6 createAddressIPv6(const in6_addr& address, uint16_t port);

// Convert an in_addr into an IPv4
void convertInaddrToIPv4(const in_addr& src, UdcAddressIPv4& dst);

// Convert an in6_addr into an IPv6
void convertInaddrToIPv6(const in6_addr& src, UdcAddressIPv6& dst);

// Send a packet
bool sendPacket(SOCKET s, const std::vector<uint8_t>& data);

// Receive a packet on an IPv4 port
// tmpBuffer is a buffer for holding temporary packet memory with size = max size of received packet
// returns 1 on success
// returns 0 if there are no messages left to receive
// returns -1 if the message was truncated due to size
// returns -2 if there was an error such as socket being closed unexpectedly
int32_t receivePacketIPv4(SOCKET s, std::vector<uint8_t>& tmpBuffer, UdcAddressIPv4& sourceIP, std::vector<uint8_t>& data);

// Receive a packet on an IPv6 port
// tmpBuffer is a buffer for holding temporary packet memory with size = max size of received packet
// returns 1 on success
// returns 0 if there are no messages left to receive
// returns -1 if the message was truncated due to size
// returns -2 if there was an error such as socket being closed unexpectedly
int32_t receivePacketIPv6(SOCKET s, std::vector<uint8_t>& tmpBuffer, UdcAddressIPv6& sourceIP, std::vector<uint8_t>& data);

#endif

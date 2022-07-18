// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_H
#define UDC_SOCKET_H

#include "udp_connect.h"

#include <cstdint>
#include <vector>
#include <string>

#ifdef OS_WINDOWS
#include <winsock2.h>
#endif

// UDP Socket wrapper for platform-specific socket calls
class UdcSocket
{
public:

    // Constructor
    UdcSocket();

    // Destructor
    ~UdcSocket();

    // Copy Consturctor
    UdcSocket(const UdcSocket&) = delete;

    // Copy-Assignment Operator
    UdcSocket& operator=(const UdcSocket&) = delete;

    // Move Constructor
    UdcSocket(UdcSocket&& o) noexcept;

    // Move Assignment Operator
    UdcSocket& operator=(UdcSocket&& o) noexcept;

    // Returns true if the socket is connected/bound
    [[nodiscard]]
    bool isConnected() const;

    // Determine IPv4 address and port from nodeName and serviceName
    [[nodiscard]]
    static bool stringToIPv4(
        const std::string& nodeName,
        const std::string& serviceName,
        UdcAddressIPv4& dstAddress,
        uint16_t& dstPort);

    // Determine IPv6 address and port from nodeName and serviceName
    [[nodiscard]]
    static bool stringToIPv6(
        const std::string& nodeName,
        const std::string& serviceName,
        UdcAddressIPv6& dstAddress,
        uint16_t& dstPort);

    // Connect to a remote node with IPv4
    // returns true on success
    [[nodiscard]]
    bool remoteConnectIPv4();

    // Connect to a remote node with IPv6
    // returns true on success
    [[nodiscard]]
    bool remoteConnectIPv6();

    // Start a connection to a local socket that can listen for IPv4 packets
    // returns true on success
    [[nodiscard]]
    bool localBindIPv4(uint16_t localPort);

    // Start a connection to a local socket that can listen for IPv6 packets
    // allowIPv4=true will try to enable dual-stack socket binding that can receive
    // both IPv4 and IPv6 packets.
    // If the option doesn't exist on the target platform,
    // and allowIPv4=true, then the local connect call will fail.
    // returns true on success
    [[nodiscard]]
    bool localBindIPv6(uint16_t localPort, bool allowIPv4 = false);

    // Disconnect from a local or remote connection
    // automatically called by destructor
    void disconnect();

    // Send a packet over IPv4
    // returns true on success
    [[nodiscard]]
    bool sendIPv4(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size) const;

    // Send a packet over IPv6
    // returns true on success
    [[nodiscard]]
    bool sendIPv6(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size) const;

    // Receive packets on a port bound with localBindIPv4
    // returns 1 on success
    // returns 0 if there are no messages left to receive
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receiveIPv4(UdcAddressIPv4& sourceIP, uint16_t& port, uint8_t* buffer, uint32_t& size) const;

    // Receive packets on a port bound with localBindIPv6
    // returns 1 on success
    // returns 0 if there are no messages left to receive
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receiveIPv6(UdcAddressIPv6& sourceIP, uint16_t& port, uint8_t* buffer, uint32_t& size) const;

protected:
#ifdef OS_WINDOWS
    SOCKET m_socket;
#endif
};

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_H
#define UDC_SOCKET_H

#include "udp_connect.h"
#include "UdcTypes.h"

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

    // Copy Consturctor
    UdcSocket(const UdcSocket&) = delete;

    // Copy-Assignment Operator
    UdcSocket& operator=(const UdcSocket&) = delete;

    // Destructor
    ~UdcSocket();

    // Returns true if the socket is connected/bound
    [[nodiscard]]
    bool isConnected() const;

    // Determine IPv6 address and port from nodeName and serviceName
    [[nodiscard]]
    static bool stringToIPv6(
        const std::string& nodeName,
        const std::string& serviceName,
        UdcAddressIPv6& dstAddress,
        uint16_t& dstPort);

    // Determine IPv4 address and port from nodeName and serviceName
    [[nodiscard]]
    static bool stringToIPv4(
        const std::string& nodeName,
        const std::string& serviceName,
        UdcAddressIPv4& dstAddress,
        uint16_t& dstPort);

    // Connect to a remote node with nodeName and serviceName
    // returns true on success
    [[nodiscard]]
    bool remoteConnect(const std::string& nodeName, const std::string& serviceName);

    // Connect to a remote node with IPv4
    // returns true on success
    [[nodiscard]]
    bool remoteConnectIPv4(const UdcAddressIPv4& remoteIp, uint16_t remotePort);

    // Connect to a remote node with IPv6
    // returns true on success
    [[nodiscard]]
    bool remoteConnectIPv6(const UdcAddressIPv6& remoteIp, uint16_t remotePort);

    // Start a connection to a local socket that can listen for IPv4 packets
    // returns true on success
    [[nodiscard]]
    bool localBindIPv4(uint16_t localPort);

    // Start a connection to a local socket that can listen for IPv6 packets
    // allowIPv4=true will try to enable dual-stack socket binding that can receiveIPv4
    // both IPv4 and IPv6 packets.
    // If the option doesn't exist on the target platform,
    // and allowIPv4=true, then the local connect call will fail.
    // returns true on success
    [[nodiscard]]
    bool localBindIPv6(uint16_t localPort, bool allowIPv4 = false);

    // Disconnect from a local or remote connection
    // automatically called by destructor
    void disconnect();

    // Send a packet
    // returns true on success
    [[nodiscard]]
    bool send(const std::vector<uint8_t>& data) const;

    // Receive packets on a port bound with localBindIPv4
    // returns 1 on success
    // returns 0 if there are no messages left to receiveIPv4
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receiveIPv4(UdcAddressIPv4& sourceIP, std::vector<uint8_t>& data, size_t maxSize = 2048) const;

    // Receive packets on a port bound with localBindIPv6
    // returns 1 on success
    // returns 0 if there are no messages left to receiveIPv4
    // returns -1 if the message was truncated due to size
    // returns -2 if there was an error such as socket being closed unexpectedly
    [[nodiscard]]
    int32_t receiveIPv6(UdcAddressIPv6& sourceIP, std::vector<uint8_t>& data, size_t maxSize = 2048) const;

protected:
#ifdef OS_WINDOWS
    SOCKET m_socket;
#endif
    static std::vector<uint8_t> m_buffer;
};

#endif

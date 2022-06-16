// udp-connect
// Kyle J Burgess

#ifndef UDC_SOCKET_H
#define UDC_SOCKET_H

#include "udp_connect.h"

#ifdef OS_WINDOWS
#include <winsock2.h>
#endif

// UDP Socket wrapper for platform-specific socket calls
class UdcSocket
{
public:

    UdcSocket();

    // Connect to a remote node with IPv4
    bool remoteConnect(UdcAddressIPv4 remoteIp, uint16_t remotePort);

    // Connect to a remote node with IPv6
    bool remoteConnect(UdcAddressIPv6 remoteIp, uint16_t remotePort);

    // Start a connection to a local socket that can listen for IPv4 packets
    bool localConnectIPv4(uint16_t localPort);

    // Start a connection to a local socket that can listen for IPv6 packets
    // allowIPv4=true will try to enable dual-stack socket binding that can receive
    // both IPv4 and IPv6 packets.
    // If the option doesn't exist on the target platform,
    // and allowIPv4=true, then the local connect call will fail.
    bool localConnectIPv6(uint16_t localPort, bool allowIPv4 = false);

    // Disconnect from a local or remote connection
    // automatically called by destructor
    void disconnect();


//    bool send();
//
//    bool receive();



protected:
#ifdef OS_WINDOWS
    SOCKET m_socket;
#endif
};

#endif

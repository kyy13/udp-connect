// udp-connect
// Kyle J Burgess

#ifndef UDC_CONNECTION_H
#define UDC_CONNECTION_H

#include "udp_connect.h"
#include "UdcEndPointId.h"

#include <cstdint>
#include <string>
#include <chrono>

struct UdcConnection
{
    UdcConnection();

    UdcEndPointId id;

    // Input address data
    std::string nodeName;
    std::string serviceName;

    // Address Data
    uint16_t port;
    UdcAddressFamily addressFamily;
    union
    {
        UdcAddressIPv4 addressIPv4;
        UdcAddressIPv6 addressIPv6;
    };

    std::chrono::milliseconds tryConnectTimeout;
    std::chrono::milliseconds tryConnectFirstTime;
    std::chrono::milliseconds tryConnectPrevTime;
};

#endif

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

    // True when first connected,
    // false if UDC_EVENT_CONNECTION_LOST
    // and set back to true when UDC_EVENT_CONNECTION_REGAINED
    bool isConnected;

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

    // tryConnect timing
    std::chrono::milliseconds tryConnectTimeout;
    std::chrono::milliseconds tryConnectFirstTime;
    std::chrono::milliseconds tryConnectPrevTime;

    // ping timing
    std::chrono::milliseconds pingValue; // the last retrieved ping value
    std::chrono::milliseconds pongPrevTime; // last time that a pong was received at
    std::chrono::milliseconds pingPrevTime; // last time a ping was sent
};

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDC_EVENT_H
#define UDC_EVENT_H

#include "udp_connect.h"

struct UdcEvent
{
    UdcEventType eventType;
    UdcEndPointId endPointId;
    UdcAddressFamily addressFamily;

    union
    {
        UdcAddressIPv4 addressIPv4;
        UdcAddressIPv6 addressIPv6;
    };

    uint16_t port;
    const uint8_t* buffer;
};

#endif

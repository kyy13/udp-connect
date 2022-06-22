// udp-connect
// Kyle J Burgess

#ifndef UDC_EVENT_H
#define UDC_EVENT_H

#include "udp_connect.h"

struct UdcEvent
{
    UdcEventType eventType;
    UdcEndPointId endPointId;
    const char* nodeName;
    const char* serviceName;
    const uint8_t* buffer;
};

#endif

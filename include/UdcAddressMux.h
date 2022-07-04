// udp-connect
// Kyle J Burgess

#ifndef UDC_ADDRESS_MUX_H
#define UDC_ADDRESS_MUX_H

#include "udp_connect.h"
#include "UdcTypes.h"

struct UdcAddressMux
{
    UdcAddressFamily family;
    union
    {
        UdcAddressIPv4 ipv4;
        UdcAddressIPv6 ipv6;
    } address;
    uint16_t port;
};

#endif

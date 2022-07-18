// udp-connect
// Kyle J Burgess

#ifndef UDC_ADDRESS_MUX_H
#define UDC_ADDRESS_MUX_H

#include "udp_connect.h"

// IP version
enum UdcAddressFamily : uint8_t
{
    UDC_IPV4,
    UDC_IPV6,
};

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

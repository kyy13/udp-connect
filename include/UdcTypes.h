// udp-connect
// Kyle J Burgess

#ifndef UDC_TYPES_H
#define UDC_TYPES_H

#include <cstdint>

// IP version
enum UdcAddressFamily : uint8_t
{
    UDC_IPV4,
    UDC_IPV6,
};

// IPv4 Address
// Stored as 4 octets in the format [0].[1].[2].[3]
struct UdcAddressIPv4
{
    uint8_t octets[4];
};

// IPv6 Address
// Stored as 8 segments in the format [0]:[1]:[2]:[3]:[4]:[5]:[6]:[7]
// where each segment is in network byte-order (Big Endian)
struct UdcAddressIPv6
{
    uint16_t segments[8];
};

#endif

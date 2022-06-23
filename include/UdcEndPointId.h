// udp-connect
// Kyle J Burgess

#ifndef UDC_ENDPOINT_ID_H
#define UDC_ENDPOINT_ID_H

#include "udp_connect.h"
#include "UdcTypes.h"

// Create a new global endpoint id from IPv4 address and port
[[nodiscard]]
UdcEndPointId newEndPointId(const UdcAddressIPv4& address, uint16_t port);

// Create a new global endpoint id from IPv6 address and port
[[nodiscard]]
UdcEndPointId newEndPointId(const UdcAddressIPv6& address, uint16_t port);

// True if the endpoint id is null (all zeros)
[[nodiscard]]
bool isNullEndPointId(const UdcEndPointId& deviceId);

// Returns true if both endpoints are equal ids
[[nodiscard]]
bool cmpEndPointId(const UdcEndPointId& a, const UdcEndPointId& b);

// Hashing function for using an endpoint id as a key in a hash table
struct UdcEndPointIdHasher
{
    uint32_t operator()(const UdcEndPointId& clientId) const;
};

// Endpoint comparator function for using an endpoint id as a key in a hash table
struct UdcEndPointIdComparator
{
    uint32_t operator()(const UdcEndPointId& a, const UdcEndPointId& b) const;
};

#endif

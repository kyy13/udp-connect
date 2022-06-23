// udp-connect
// Kyle J Burgess

#include "UdcEndPointId.h"

#include <cstring>

UdcEndPointId newEndPointId(const UdcAddressIPv4& address, uint16_t port)
{
    UdcEndPointId id {};
    UdcAddressFamily family = UDC_IPV4;

    memcpy(id.bytes, address.octets, sizeof(address.octets));
    memcpy(id.bytes + 16, &port, sizeof(port));
    memcpy(id.bytes + 18, &family, sizeof(family));

    return id;
}

UdcEndPointId newEndPointId(const UdcAddressIPv6& address, uint16_t port)
{
    UdcEndPointId id {};
    UdcAddressFamily family = UDC_IPV6;

    memcpy(id.bytes, address.segments, sizeof(address.segments));
    memcpy(id.bytes + 16, &port, sizeof(port));
    memcpy(id.bytes + 18, &family, sizeof(family));

    return id;
}

bool isNullEndPointId(const UdcEndPointId& deviceId)
{
    static UdcEndPointId nulLDeviceId {};
    return memcmp(deviceId.bytes, nulLDeviceId.bytes, sizeof(deviceId.bytes)) == 0;
}

bool cmpEndPointId(const UdcEndPointId& a, const UdcEndPointId& b)
{
    return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

uint32_t UdcEndPointIdHasher::operator()(const UdcEndPointId& clientId) const
{
    return
        *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 0]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 4]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 8]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[12]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[16]);
}

uint32_t UdcEndPointIdComparator::operator()(const UdcEndPointId& a, const UdcEndPointId& b) const
{
    return cmpEndPointId(a, b);
}

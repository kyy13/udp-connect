// udp-connect
// Kyle J Burgess

#include "UdcDeviceId.h"

#include <cstring>

UdcDeviceId newDeviceId(const UdcAddressIPv4& address, uint16_t port)
{
    UdcDeviceId id {};
    UdcAddressFamily family = UDC_IPV4;

    memcpy(id.bytes, address.octets, sizeof(address.octets));
    memcpy(id.bytes + 16, &port, sizeof(port));
    memcpy(id.bytes + 18, &family, sizeof(family));

    return id;
}

UdcDeviceId newDeviceId(const UdcAddressIPv6& address, uint16_t port)
{
    UdcDeviceId id {};
    UdcAddressFamily family = UDC_IPV6;

    memcpy(id.bytes, address.segments, sizeof(address.segments));
    memcpy(id.bytes + 16, &port, sizeof(port));
    memcpy(id.bytes + 18, &family, sizeof(family));

    return id;
}

bool isNullDeviceId(const UdcDeviceId& deviceId)
{
    static UdcDeviceId nulLDeviceId {};
    return memcmp(deviceId.bytes, nulLDeviceId.bytes, sizeof(deviceId.bytes)) == 0;
}

bool cmpDeviceId(const UdcDeviceId& a, const UdcDeviceId& b)
{
    return memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

uint32_t UdcDeviceIdHasher::operator()(const UdcDeviceId& clientId) const
{
    return
        *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 0]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 4]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[ 8]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[12]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[16]);
}

uint32_t UdcDeviceIdComparator::operator()(const UdcDeviceId& a, const UdcDeviceId& b) const
{
    return cmpDeviceId(a, b);
}

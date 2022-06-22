// udp-connect
// Kyle J Burgess

#ifndef UDC_DEVICE_ID_H
#define UDC_DEVICE_ID_H

#include "udp_connect.h"
#include "UdcTypes.h"

[[nodiscard]]
UdcDeviceId newDeviceId(const UdcAddressIPv4& address, uint16_t port);

[[nodiscard]]
UdcDeviceId newDeviceId(const UdcAddressIPv6& address, uint16_t port);

[[nodiscard]]
bool isNullDeviceId(const UdcDeviceId& deviceId);

[[nodiscard]]
bool cmpDeviceId(const UdcDeviceId& a, const UdcDeviceId& b);

struct UdcDeviceIdHasher
{
    uint32_t operator()(const UdcDeviceId& clientId) const;
};

struct UdcDeviceIdComparator
{
    uint32_t operator()(const UdcDeviceId& a, const UdcDeviceId& b) const;
};

#endif

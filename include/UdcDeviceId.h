// udp-connect
// Kyle J Burgess

#ifndef UDC_DEVICE_ID_H
#define UDC_DEVICE_ID_H

#include "udp_connect.h"
#include "UdcTypes.h"

[[nodiscard]]
UdcEndPointId newDeviceId(const UdcAddressIPv4& address, uint16_t port);

[[nodiscard]]
UdcEndPointId newDeviceId(const UdcAddressIPv6& address, uint16_t port);

[[nodiscard]]
bool isNullDeviceId(const UdcEndPointId& deviceId);

[[nodiscard]]
bool cmpDeviceId(const UdcEndPointId& a, const UdcEndPointId& b);

struct UdcDeviceIdHasher
{
    uint32_t operator()(const UdcEndPointId& clientId) const;
};

struct UdcDeviceIdComparator
{
    uint32_t operator()(const UdcEndPointId& a, const UdcEndPointId& b) const;
};

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDC_COMMON_H
#define UDC_COMMON_H

#include "udp_connect.h"

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

bool setSocketNonBlocking(SOCKET socket);
bool setSocketReuseAddress(SOCKET socket);

in_addr convertFromIPv4(const UdcAddressIPv4& address);
void convertToIPv4(const in_addr& src, UdcAddressIPv4& dst);

in6_addr convertFromIPv6(const UdcAddressIPv6& address);
void convertToIPv6(const in6_addr& src, UdcAddressIPv6& dst);

#endif

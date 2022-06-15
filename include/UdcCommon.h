// udp-connect
// Kyle J Burgess

#ifndef UDC_COMMON_H
#define UDC_COMMON_H

#include "udp_connect.h"

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

bool setSocketNonBlocking(SOCKET socket);

in_addr convertIPv4(IpAddress& address);

in6_addr convertIPv6(IpAddress& address);

#endif

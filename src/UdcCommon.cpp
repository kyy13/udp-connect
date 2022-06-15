// udp-connect
// Kyle J Burgess

#include "UdcCommon.h"

#include <cassert>
#include <cstring>

bool setSocketNonBlocking(SOCKET socket)
{
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) != SOCKET_ERROR;
}

in_addr convertIPv4(IpAddress& address)
{
    in_addr result;
    memcpy(&result.S_un.S_un_b.s_b1, address.ip_v4, sizeof(address.ip_v4));
    return result;
}

in6_addr convertIPv6(IpAddress& address)
{
    in6_addr result;
    memcpy(result.u.Byte, address.ip_v6, sizeof(address.ip_v6));
    return result;
}

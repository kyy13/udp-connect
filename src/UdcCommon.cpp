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

bool setSocketReuseAddress(SOCKET socket)
{
    int opt = 1;
    return setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) != SOCKET_ERROR;
}

in_addr convertFromIPv4(const UdcAddressIPv4& address)
{
    in_addr result {};
    memcpy(&result.S_un.S_un_b.s_b1, address.octets, sizeof(address.octets));
    return result;
}

void convertToIPv4(const in_addr& src, UdcAddressIPv4& dst)
{
    memcpy(dst.octets, &src.S_un.S_un_b.s_b1, sizeof(dst.octets));
}

in6_addr convertFromIPv6(const UdcAddressIPv6& address)
{
    in6_addr result {};
    memcpy(result.u.Byte, address.segments, sizeof(address.segments));
    return result;
}

void convertToIPv6(const in6_addr& src, UdcAddressIPv6& dst)
{
    memcpy(dst.segments, src.u.Byte, sizeof(dst.segments));
}

// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include <cstring>
#include <ws2tcpip.h>

bool udcStringToIPv4(const char* ipString, UdcAddressIPv4* ipAddress)
{
    in_addr address {};

    if (inet_pton(AF_INET, ipString, &address) == 1)
    {
        memcpy(ipAddress->octets, &address.S_un.S_un_b.s_b1, sizeof(ipAddress->octets));
        return true;
    }

    return false;
}

bool udcStringToIPv6(const char* ipString, UdcAddressIPv6* ipAddress)
{
    in6_addr address {};

    if (inet_pton(AF_INET6, ipString, &address) == 1)
    {
        memcpy(ipAddress->segments, &address.u.Byte, sizeof(ipAddress->segments));
        return true;
    }

    return false;
}

uint32_t udcIPv4ToString(const UdcAddressIPv4* ipAddress, char* ipString)
{
    char buffer[INET_ADDRSTRLEN];
    in_addr address {};
    size_t size;

    memcpy(&address.S_un.S_un_b.s_b1, ipAddress->octets, sizeof(ipAddress->octets));
    inet_ntop(AF_INET,&address,buffer, sizeof(buffer));

    size = strlen(buffer);
    if (ipString != nullptr)
    {
        memcpy(ipString, buffer, size);
    }

    return size;
}

uint32_t udcIPv6ToString(const UdcAddressIPv6* ipAddress, char* ipString)
{
    char buffer[INET6_ADDRSTRLEN];
    in6_addr address {};
    size_t size;

    memcpy(&address.u.Byte, ipAddress->segments, sizeof(ipAddress->segments));
    inet_ntop(AF_INET6,&address,buffer, sizeof(buffer));

    size = strlen(buffer);
    if (ipString != nullptr)
    {
        memcpy(ipString, buffer, size);
    }

    return size;
}

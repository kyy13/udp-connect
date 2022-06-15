// udp-connect
// Kyle J Burgess

#include "udp_connect.h"

#include <cstring>
#include <ws2tcpip.h>

bool udcConvertStringToIp(const char* str, IpVersion* version, IpAddress* address)
{
    union
    {
        in_addr ina4;
        in6_addr ina6;
    };

    // Try to convert to IPv6
    if (inet_pton(AF_INET6, str, &ina6) == 1)
    {
        *version = IP_V6;
        memcpy(address->ip_v6, &ina6.u.Byte, sizeof(address->ip_v6));
        return true;
    }

    // Try to convert to IPv4
    if (inet_pton(AF_INET, str, &ina4) == 1)
    {
        *version = IP_V4;
        memcpy(address->ip_v4, &ina4.S_un.S_un_b.s_b1, sizeof(address->ip_v4));
        return true;
    }

    // Failed to convert
    return false;
}

uint32_t udcConvertIpToString(IpVersion version, IpAddress address, char* str)
{
    union
    {
        char buffer4[INET_ADDRSTRLEN];
        char buffer6[INET6_ADDRSTRLEN];
    };

    union
    {
        in_addr ina4;
        in6_addr ina6;
    };

    // IPv4
    if (version == IP_V4)
    {
        memcpy(&ina4.S_un.S_un_b.s_b1, address.ip_v4, sizeof(address.ip_v4));
        inet_ntop(AF_INET,&ina4,buffer4, sizeof(buffer4));

        size_t size = strlen(buffer4);
        if (str != nullptr)
        {
            memcpy(str, buffer4, size);
        }

        return size;
    }

    // IPv6
    memcpy(&ina6.u.Byte, address.ip_v6, sizeof(address.ip_v6));
    inet_ntop(AF_INET6,&ina6,buffer6, sizeof(buffer6));

    size_t size = strlen(buffer6);
    if (str != nullptr)
    {
        memcpy(str, buffer6, size);
    }

    return size;
}

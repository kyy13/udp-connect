// udp-connect
// Kyle J Burgess

#include "UdcCommon.h"

#include <cassert>
#include <ws2tcpip.h>

bool setSocketNonBlocking(SOCKET socket)
{
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) != SOCKET_ERROR;
}

void convertToIp(const sockaddr_in* sa, std::string& ip)
{
    if (sa->sin_family == AF_INET)
    {
        char buffer[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &sa->sin_addr,
            buffer,
            sizeof(buffer));

        size_t size = strlen(buffer);
        assert(size <= INET_ADDRSTRLEN);
        ip.assign(buffer, buffer + size);
    }
    else
    {
        char buffer[INET6_ADDRSTRLEN];

        inet_ntop(
            AF_INET6,
            &sa->sin_addr,
            buffer,
            sizeof(buffer));

        size_t size = strlen(buffer);
        assert(size <= INET6_ADDRSTRLEN);
        ip.assign(buffer, buffer + size);
    }
}

// udp-connect
// Kyle J Burgess

#include "UdcCommon.h"

bool setSocketNonBlocking(SOCKET socket)
{
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) != SOCKET_ERROR;
}

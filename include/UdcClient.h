// udp-connect
// Kyle J Burgess

#ifndef UDC_CLIENT_H
#define UDC_CLIENT_H

#include "udp_connect.h"
#include "UdcSocketSender.h"

struct UdcClient
{
    UdcSocketSender socket;
};

#endif

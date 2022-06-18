// udp-connect
// Kyle J Burgess

#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "udp_connect.h"
#include "UdcSocketReceiver.h"

#include <vector>

struct UdcServer
{
    UdcSocketReceiver socket;
    std::vector<UdcAddress> clients;
};

#endif

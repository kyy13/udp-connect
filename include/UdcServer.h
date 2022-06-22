// udp-connect
// Kyle J Burgess

#ifndef UDC_SERVER_H
#define UDC_SERVER_H

#include "udp_connect.h"
#include "UdcDualSocket.h"
#include "UdcDeviceId.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <queue>

struct UdcClientInfo
{
    UdcDeviceId id;

    uint16_t port;

    UdcAddressFamily addressFamily;

    union
    {
        UdcAddressIPv4 addressIPv4;
        UdcAddressIPv6 addressIPv6;
    };

    UdcStatus status;

    std::chrono::milliseconds timeout;
    std::chrono::milliseconds tryConnectTime;
    std::chrono::milliseconds lastSendTime;
};

struct UdcServer
{
    UdcDeviceId id;

    UdcSignature signature;

    UdcDualSocket socket;

    // Maps temporary device ID to clients pending connection
    std::queue<std::unique_ptr<UdcClientInfo>> pendingClients;

    // Maps device ID to connected clients
    std::unordered_map<UdcDeviceId, std::unique_ptr<UdcClientInfo>, UdcDeviceIdHasher, UdcDeviceIdComparator> clients;

    // Message Buffer
    std::vector<uint8_t> messageBuffer;
};

#endif

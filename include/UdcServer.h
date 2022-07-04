// udp-connect
// Kyle J Burgess

#ifndef UDC_SERVER_H
#define UDC_SERVER_H

#include "udp_connect.h"
#include "UdcSocketMux.h"
#include "UdcPacketLogger.h"
#include "UdcClient.h"
#include "UdcEvent.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <queue>

struct UdcServer
{
    UdcServer();

    UdcServer(const std::string& logFileName);

    UdcEndPointId idCounter;

    UdcSignature signature;

    UdcSocketMux socket;

    UdcEvent event;

    // Maps temporary device ID to clients pending connection
    std::queue<std::unique_ptr<UdcClient>> pendingClients;

    // Maps device ID to connected clients
    std::unordered_map<UdcEndPointId, std::unique_ptr<UdcClient>> clients;

    // Maps IP/port to endpointId


    // Message Buffer
    uint8_t* buffer;
    uint32_t bufferSize;
};

#endif

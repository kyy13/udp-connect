// udp-connect
// Kyle J Burgess

#ifndef UDC_SERVER_H
#define UDC_SERVER_H

#include "udp_connect.h"
#include "UdcSocketMux.h"
#include "UdcPacketLogger.h"
#include "UdcConnection.h"
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
    std::queue<std::unique_ptr<UdcConnection>> pendingClients;

    // Maps device ID to connected clients
    std::unordered_map<UdcEndPointId, std::unique_ptr<UdcConnection>> clients;

    // Message Buffer
    std::vector<uint8_t> messageBuffer;
};

#endif

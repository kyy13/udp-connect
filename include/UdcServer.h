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

class UdcServerImpl
{
public:

    UdcServerImpl(UdcSignature signature, uint16_t portIPv4, uint16_t portIPv6, uint8_t* buffer, uint32_t bufferSize);

    UdcServerImpl(UdcSignature signature, uint16_t portIPv4, uint16_t portIPv6, uint8_t* buffer, uint32_t bufferSize, const std::string& logFileName);

    [[nodiscard]]
    UdcEndPointId createUniqueId();

    void addPendingClient(std::unique_ptr<UdcClient> client, std::chrono::milliseconds time);

    void sendUnreliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size);

    [[nodiscard]]
    const UdcEvent* receiveMessages(std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* updatePendingClients(std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* updateClientConnectionStatus(std::chrono::milliseconds time);

    void processConnectionRequest(const UdcAddressMux& fromAddress);

    [[nodiscard]]
    const UdcEvent* processConnectionHandshake(const UdcAddressMux& fromAddress, std::chrono::milliseconds time);

    void processPing(const UdcAddressMux& fromAddress);

    [[nodiscard]]
    const UdcEvent* processPong(const UdcAddressMux& fromAddress, std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* processUnreliable(const UdcAddressMux& fromAddress, uint32_t msgSize);

    [[nodiscard]]
    bool tryGetClient(UdcEndPointId clientId, UdcClient** client);

    [[nodiscard]]
    bool tryGetFirstPendingClient(UdcClient** client);

protected:

    UdcSocketMux m_socket;

    UdcEndPointId m_idCounter;

    UdcSignature m_packetSignature;

    UdcEvent m_eventBuffer;

    // Maps temporary device ID to clients pending connection
    std::queue<std::unique_ptr<UdcClient>> m_pendingClients;

    // Maps device ID to connected clients
    std::unordered_map<UdcEndPointId, std::unique_ptr<UdcClient>> m_clients;

    // Message Buffer
    uint8_t* m_messageBuffer;
    uint32_t m_messageBufferSize;
};

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDC_SERVER_H
#define UDC_SERVER_H

#include "udp_connect.h"

#include "UdcSocketMux.h"
#include "UdcPacketLogger.h"
#include "UdcAddressHash.h"
#include "UdcClient.h"
#include "UdcEvent.h"

#include <memory>
#include <chrono>
#include <deque>

class UdcServerImpl
{
public:

    UdcServerImpl(UdcSignature signature, uint8_t* buffer, uint32_t bufferSize);

    UdcServerImpl(UdcSignature signature, uint8_t* buffer, uint32_t bufferSize, const std::string& logFileName);

    [[nodiscard]]
    bool tryBindIPv4(uint16_t port);

    [[nodiscard]]
    bool tryBindIPv6(uint16_t port);

    [[nodiscard]]
    UdcEndPointId createUniqueId();

    [[nodiscard]]
    bool getEndPointStatus(UdcEndPointId id, std::chrono::milliseconds& ping);

    void addPendingClient(std::shared_ptr<UdcClient> client, std::chrono::milliseconds time);

    void disconnectFromClient(UdcEndPointId endPointId);

    [[nodiscard]]
    bool sendUnreliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size);

    [[nodiscard]]
    bool sendReliableMessage(UdcEndPointId endPointId, const uint8_t* data, uint32_t size);

    [[nodiscard]]
    const UdcEvent* receiveMessages(std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* updatePendingClients(std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* updateClientConnectionStatus(std::chrono::milliseconds time);

protected:

    UdcSocketMux m_socket;

    UdcEndPointId m_idCounter;

    UdcSignature m_packetSignature;

    UdcEvent m_eventBuffer;

    // Maps temporary device ID to clients pending connection
    std::deque<std::shared_ptr<UdcClient>> m_pendingClients;

    // Maps device ID to connected clients
    std::unordered_map<UdcEndPointId, std::shared_ptr<UdcClient>> m_clientsById;

    // Maps address to connected clients
    UdcAddressMap<std::shared_ptr<UdcClient>> m_clientsByAddress;

    // Maps address to client reliable states
    UdcAddressMap<int> m_reliableStates;

    // Message Buffer
    uint8_t* m_messageBuffer;
    uint32_t m_messageBufferSize;

    void processConnectionRequest(const UdcAddressMux& fromAddress);

    [[nodiscard]]
    const UdcEvent* processConnectionHandshake(const UdcAddressMux& fromAddress, std::chrono::milliseconds time);

    void processPing(const UdcAddressMux& fromAddress);

    [[nodiscard]]
    const UdcEvent* processPong(const UdcAddressMux& fromAddress, std::chrono::milliseconds time);

    [[nodiscard]]
    const UdcEvent* processUnreliable(const UdcAddressMux& fromAddress, uint32_t msgSize);

    [[nodiscard]]
    const UdcEvent* processReliableMessage(int state, const UdcAddressMux& fromAddress, uint32_t size);

    [[nodiscard]]
    const UdcEvent* processReliableHandshake(int state, const UdcAddressMux& fromAddress, std::chrono::milliseconds time);

    [[nodiscard]]
    bool tryGetClient(UdcEndPointId clientId, UdcClient** client);

    [[nodiscard]]
    bool tryGetClient(const UdcAddressMux& address, UdcClient** client);

    [[nodiscard]]
    bool tryGetFirstPendingClient(UdcClient** client);
};

#endif

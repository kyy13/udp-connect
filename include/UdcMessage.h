// udp-connect
// Kyle J Burgess

#ifndef UDC_MESSAGE_H
#define UDC_MESSAGE_H

#include "udp_connect.h"

#include <cstdint>
#include <vector>

enum UdcMessageId : uint8_t
{
    UDC_MSG_CONNECTION_REQUEST,
    UDC_MSG_CONNECTION_HANDSHAKE,
    UDC_MSG_PING,
    UDC_MSG_PONG,
    UDC_MSG_EXTERNAL,
};

struct UdcMsgConnection
{
    UdcEndPointId clientId;
};

//struct UdcMsgExternal
//{
//    UdcEndPointId clientId;
//};

struct UdcMsgPingPong
{
    UdcEndPointId clientId;
    uint32_t timeOnServer;
};

constexpr size_t UDC_MSG_HEADER_SIZE =
    sizeof(UdcSignature) +
    sizeof(UdcMessageId);

constexpr size_t UDC_MSG_CONNECTION_SIZE =
    UDC_MSG_HEADER_SIZE +
    sizeof(UdcEndPointId);

constexpr size_t UDC_MSG_EXTERNAL_SIZE =
    UDC_MSG_HEADER_SIZE;

constexpr size_t UDC_MSG_PING_PONG_SIZE =
    UDC_MSG_HEADER_SIZE +
    sizeof(UdcMsgPingPong::clientId) +
    sizeof(UdcMsgPingPong::timeOnServer);

void udcGenerateHeader(uint8_t* buffer, uint32_t& size, UdcSignature signature, UdcMessageId messageId);

[[nodiscard]]
bool udcReadHeader(const uint8_t* buffer, uint32_t size, uint32_t signature, UdcMessageId& messageId);

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const UdcMsgConnection& body, UdcSignature signature, UdcMessageId messageId);

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const UdcMsgPingPong& body, UdcSignature signature, UdcMessageId messageId);

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const uint8_t* body, uint32_t bodySize, UdcSignature signature, UdcMessageId messageId); // External

[[nodiscard]]
bool udcReadMessage(const uint8_t* buffer, uint32_t size, UdcMsgConnection& dst);

[[nodiscard]]
bool udcReadMessage(const uint8_t* buffer, uint32_t size, UdcMsgPingPong& dst);

[[nodiscard]]
bool udcReadMessage(const uint8_t* buffer, uint32_t size); // External

#endif

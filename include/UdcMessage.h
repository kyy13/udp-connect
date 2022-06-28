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

// Message header
struct UdcMessage
{
    // Size of the deserialized message in bytes
    static constexpr uint32_t SIZE =
        sizeof(UdcSignature) +
        sizeof(UdcMessageId);

    // Message signature
    // for recognizing the message as a udp-connect message
    UdcSignature msgSig;

    // Message ID
    UdcMessageId msgId;

    // serialize message into a buffer
    // input size is the maximum size in bytes of the buffer
    // output size is the size of the serialized data
    // returns false if the buffer isn't large enough
    [[nodiscard]]
    bool serialize(uint8_t* buffer, uint32_t& size) const;

    // deserialize a buffer into the message
    // returns false if the msgSize is wrong
    // or the bufferSize isn't large enough
    [[nodiscard]]
    bool deserialize(const uint8_t* buffer, uint32_t bufferSize, uint32_t msgSize);
};

struct UdcConnectionRequest : UdcMessage
{
    // Size of the deserialized message in bytes
    static constexpr uint32_t SIZE =
        UdcMessage::SIZE +
        sizeof(UdcEndPointId);

    // Local ID of the client
    UdcEndPointId endPointId;

    // serialize message into a buffer
    // input size is the maximum size in bytes of the buffer
    // output size is the size of the serialized data
    // returns false if the buffer isn't large enough
    [[nodiscard]]
    bool serialize(uint8_t* buffer, uint32_t& size) const;

    // deserialize a buffer into the message
    // returns false if the msgSize is wrong
    // or the bufferSize isn't large enough
    [[nodiscard]]
    bool deserialize(const uint8_t* buffer, uint32_t bufferSize, uint32_t msgSize);
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

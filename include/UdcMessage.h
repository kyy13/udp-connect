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
    UDC_MSG_UNRELIABLE,
    UDC_MSG_RELIABLE_ANY,
    UDC_MSG_RELIABLE_0,
    UDC_MSG_RELIABLE_1,
    UDC_MSG_RELIABLE_HANDSHAKE_ANY,
    UDC_MSG_RELIABLE_HANDSHAKE_0,
    UDC_MSG_RELIABLE_HANDSHAKE_1,
};

namespace serial
{
    // Header (5 bytes)
    // Signature (4 bytes)
    // MessageID (1 byte)
    namespace msgHeader
    {
        // Size of the deserialized message in bytes
        constexpr uint32_t SIZE =
            sizeof(UdcSignature::bytes) +
            sizeof(UdcMessageId);

        void serializeMsgSignature(uint8_t* msgBuffer, const UdcSignature& msgSignature);

        void serializeMsgId(uint8_t* msgBuffer, UdcMessageId msgId);

        void deserializeMsgSignature(const uint8_t* msgBuffer, UdcSignature& msgSignature);

        void deserializeMsgId(const uint8_t* msgBuffer, UdcMessageId& msgId);
    }

    // UDC_MSG_CONNECTION_REQUEST
    // UDC_MSG_CONNECTION_HANDSHAKE
    namespace msgConnection
    {
        // Size of the deserialized message in bytes
        constexpr uint32_t SIZE =
            msgHeader::SIZE +
            sizeof(UdcEndPointId);

        void serializeEndPointId(uint8_t* msgBuffer, UdcEndPointId endPointId);

        void deserializeEndPointId(const uint8_t* msgBuffer, UdcEndPointId& endPointId);
    }

    // UDC_MSG_PING
    // UDC_MSG_PONG
    namespace msgPingPong
    {
        // Size of the deserialized message in bytes
        constexpr uint32_t SIZE =
            msgHeader::SIZE +
            sizeof(uint32_t);

        void serializeTimeStamp(uint8_t* msgBuffer, uint32_t timeStamp);

        void deserializeTimeStamp(const uint8_t* msgBuffer, uint32_t& timeStamp);
    }

    // UDC_MSG_UNRELIABLE
    namespace msgUnreliable
    {
        // Size of minimum deserialized message in bytes
        constexpr uint32_t SIZE =
            msgHeader::SIZE;

        void serializeData(uint8_t* msgBuffer, const uint8_t* data, uint32_t dataSize);

        void deserializeData(const uint8_t* msgBuffer, uint8_t* data, uint32_t dataSize);
    }

    namespace msgReliable
    {
        // Size of minimum deserialized message in bytes
        constexpr uint32_t SIZE =
            msgHeader::SIZE +
            sizeof(uint32_t);

        void serializeTimeStamp(uint8_t* msgBuffer, uint32_t timeStamp);

        void deserializeTimeStamp(const uint8_t* msgBuffer, uint32_t& timeStamp);

        void serializeData(uint8_t* msgBuffer, const uint8_t* data, uint32_t dataSize);

        void deserializeData(const uint8_t* msgBuffer, uint8_t* data, uint32_t dataSize);
    }
}

#endif

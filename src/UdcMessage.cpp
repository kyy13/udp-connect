// udp-connect
// Kyle J Burgess

#include "UdcMessage.h"

#include <cassert>
#include <cstring>
#include <winsock2.h>

namespace serial
{
    namespace msgHeader
    {
        void serializeMsgSignature(uint8_t* msgBuffer, const UdcSignature& msgSignature)
        {
            memcpy(msgBuffer, &msgSignature.bytes, sizeof(UdcSignature::bytes));
        }

        void serializeMsgId(uint8_t* msgBuffer, UdcMessageId msgId)
        {
            memcpy(msgBuffer + sizeof(UdcSignature::bytes), &msgId, sizeof(UdcMessageId));
        }

        void deserializeMsgSignature(const uint8_t* msgBuffer, UdcSignature& msgSignature)
        {
            memcpy(&msgSignature.bytes, msgBuffer, sizeof(UdcSignature::bytes));
        }

        void deserializeMsgId(const uint8_t* msgBuffer, UdcMessageId& msgId)
        {
            memcpy(&msgId, msgBuffer + sizeof(UdcSignature::bytes), sizeof(UdcMessageId));
        }
    }

    namespace msgConnection
    {
        void serializeEndPointId(uint8_t* msgBuffer, UdcEndPointId endPointId)
        {
            memcpy(msgBuffer + msgHeader::SIZE, &endPointId, sizeof(UdcEndPointId));
        }

        void deserializeEndPointId(const uint8_t* msgBuffer, UdcEndPointId& endPointId)
        {
            memcpy(&endPointId, msgBuffer + msgHeader::SIZE, sizeof(UdcEndPointId));
        }
    }

    namespace msgPingPong
    {
        void serializeTimeStamp(uint8_t* msgBuffer, uint32_t timeStamp)
        {
            memcpy(msgBuffer + msgHeader::SIZE, &timeStamp, sizeof(timeStamp));
        }

        void deserializeTimeStamp(const uint8_t* msgBuffer, uint32_t& timeStamp)
        {
            memcpy(&timeStamp, msgBuffer + msgHeader::SIZE, sizeof(timeStamp));
        }
    }

    namespace msgUnreliable
    {
        void serializeData(uint8_t* msgBuffer, const uint8_t* data, uint32_t dataSize)
        {
            memcpy(msgBuffer + msgHeader::SIZE, data, dataSize);
        }

        void deserializeData(const uint8_t* msgBuffer, uint8_t* data, uint32_t dataSize)
        {
            memcpy(data, msgBuffer + msgHeader::SIZE, dataSize);
        }
    }

    namespace msgReliable
    {
        void serializeTimeStamp(uint8_t* msgBuffer, uint32_t timeStamp)
        {
            memcpy(msgBuffer + msgHeader::SIZE, &timeStamp, sizeof(timeStamp));
        }

        void deserializeTimeStamp(const uint8_t* msgBuffer, uint32_t& timeStamp)
        {
            memcpy(&timeStamp, msgBuffer + msgHeader::SIZE, sizeof(timeStamp));
        }

        void serializeData(uint8_t* msgBuffer, const uint8_t* data, uint32_t dataSize)
        {
            memcpy(msgBuffer + SIZE, data, dataSize);
        }

        void deserializeData(const uint8_t* msgBuffer, uint8_t* data, uint32_t dataSize)
        {
            memcpy(data, msgBuffer + SIZE, dataSize);
        }
    }
}

// udp-connect
// Kyle J Burgess

#include "UdcMessage.h"

#include <cassert>
#include <cstring>
#include <winsock2.h>

bool UdcMessage::serialize(uint8_t* buffer, uint32_t& size) const
{
    if (size < SIZE)
    {
        return false;
    }

    uint32_t sig = htonl(msgSig);

    memcpy(buffer + 0, &sig, sizeof(UdcSignature));
    memcpy(buffer + 4, &msgId, sizeof(UdcMessageId));

    size = SIZE;
    return true;
}

bool UdcMessage::deserialize(const uint8_t* buffer, uint32_t bufferSize, uint32_t msgSize)
{
    if (bufferSize < msgSize)
    {
        return false;
    }

    if (msgSize != SIZE)
    {
        return false;
    }

    uint32_t sig;

    memcpy(&sig, buffer + 0, sizeof(UdcSignature));
    memcpy(&msgId, buffer + 4, sizeof(UdcMessageId));

    msgSig = htonl(sig);

    return true;
}

bool UdcConnectionRequest::serialize(uint8_t* buffer, uint32_t& size) const
{
    if (size < SIZE)
    {
        return false;
    }

    if (!UdcMessage::serialize(buffer, size))
    {
        return false;
    }

    // NOTE: endpointId doesn't need to be in network-endian order,
    // because it is relayed exactly as-is back to the sender
    memcpy(buffer + size, &endPointId, sizeof(UdcEndPointId));

    size = SIZE;
    return true;
}

bool UdcConnectionRequest::deserialize(const uint8_t* buffer, uint32_t bufferSize, uint32_t msgSize)
{
    if (bufferSize < msgSize)
    {
        return false;
    }

    if (msgSize != SIZE)
    {
        return false;
    }

    if (!UdcMessage::deserialize(buffer, bufferSize, UdcMessage::SIZE))
    {
        return false;
    }

    // NOTE: endpointId doesn't need to be in network-endian order,
    // because it is relayed exactly as-is back to the sender
    memcpy(&endPointId, buffer + UdcMessage::SIZE, sizeof(UdcEndPointId));

    return true;
}

void udcGenerateHeader(uint8_t* buffer, uint32_t& size, UdcSignature signature, UdcMessageId messageId)
{
    assert(size >= UDC_MSG_HEADER_SIZE);

    signature = htonl(signature);

    memcpy(buffer + 0, &signature, sizeof(signature));
    memcpy(buffer + 4, &messageId, sizeof(messageId));

    size = UDC_MSG_HEADER_SIZE;
}

bool udcReadHeader(const uint8_t* buffer, uint32_t size, uint32_t signature, UdcMessageId& messageId)
{
    if (size < UDC_MSG_HEADER_SIZE)
    {
        return false;
    }

    uint32_t sigBytes;

    memcpy(&sigBytes, buffer + 0, sizeof(sigBytes));
    memcpy(&messageId, buffer + 4, sizeof(messageId));

    return htonl(sigBytes) == signature;
}

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const UdcMsgConnection& body, UdcSignature signature, UdcMessageId messageId)
{
    assert(size >= UDC_MSG_CONNECTION_SIZE);
    udcGenerateHeader(buffer, size, signature, messageId);

    memcpy(buffer + size, &body.clientId, sizeof(UdcEndPointId));
    size += sizeof(UdcEndPointId);

}

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const UdcMsgPingPong& body, UdcSignature signature, UdcMessageId messageId)
{
    assert(size >= UDC_MSG_PING_PONG_SIZE);

    udcGenerateHeader(buffer, size, signature, messageId);

    memcpy(buffer + size, &body.clientId, sizeof(UdcEndPointId));
    size += sizeof(UdcEndPointId);

    uint32_t time = htonl(body.timeOnServer);
    memcpy(buffer + size, &time, sizeof(UdcMsgPingPong::timeOnServer));
    size += sizeof(UdcMsgPingPong::timeOnServer);
}

void udcGenerateMessage(uint8_t* buffer, uint32_t& size, const uint8_t* body, uint32_t bodySize, UdcSignature signature, UdcMessageId messageId)
{
    assert(size >= UDC_MSG_EXTERNAL_SIZE);

    udcGenerateHeader(buffer, size, signature, messageId);

    memcpy(buffer + size, body, bodySize);
    size += bodySize;
}

bool udcReadMessage(const uint8_t* buffer, uint32_t size, UdcMsgConnection& dst)
{
    if (size != UDC_MSG_CONNECTION_SIZE)
    {
        return false;
    }

    const uint8_t* ptr = buffer + UDC_MSG_HEADER_SIZE;

    memcpy(&dst.clientId, ptr, sizeof(UdcEndPointId));

    return true;
}

bool udcReadMessage(const uint8_t* buffer, uint32_t size)
{
    if (size <= UDC_MSG_EXTERNAL_SIZE)
    {
        return false;
    }

    return true;
}

bool udcReadMessage(const uint8_t* buffer, uint32_t size, UdcMsgPingPong& dst)
{
    if (size != UDC_MSG_PING_PONG_SIZE)
    {
        return false;
    }

    const uint8_t* ptr = buffer + UDC_MSG_HEADER_SIZE;

    memcpy(&dst.clientId, ptr, sizeof(UdcEndPointId));
    ptr += sizeof(UdcEndPointId);

    uint32_t time;
    memcpy(&time, ptr, sizeof(UdcMsgPingPong::timeOnServer));
    dst.timeOnServer = htonl(time);

    return true;
}

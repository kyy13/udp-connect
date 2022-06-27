// udp-connect
// Kyle J Burgess

#include "UdcMessage.h"

#include <cassert>
#include <cstring>
#include <winsock2.h>

void udcGenerateHeader(std::vector<uint8_t>& msg, UdcSignature signature, UdcMessageId messageId)
{
    assert(msg.size() >= UDC_MSG_HEADER_SIZE);

    signature = htonl(signature);

    memcpy(msg.data() + 0, &signature, sizeof(signature));
    memcpy(msg.data() + 4, &messageId, sizeof(messageId));
}

bool udcReadHeader(const std::vector<uint8_t>& src, uint32_t signature, UdcMessageId& messageId)
{
    if (src.size() < UDC_MSG_HEADER_SIZE)
    {
        return false;
    }

    uint32_t sigBytes;

    memcpy(&sigBytes, src.data() + 0, sizeof(sigBytes));
    memcpy(&messageId, src.data() + 4, sizeof(messageId));

    return htonl(sigBytes) == signature;
}

void udcGenerateMessage(std::vector<uint8_t>& msg, const UdcMsgConnection& body, UdcSignature signature, UdcMessageId messageId)
{
    msg.resize(UDC_MSG_CONNECTION_SIZE);
    udcGenerateHeader(msg, signature, messageId);

    uint8_t* ptr = msg.data() + UDC_MSG_HEADER_SIZE;

    memcpy(ptr, &body.clientId, sizeof(UdcEndPointId));
}

void udcGenerateMessage(std::vector<uint8_t>& msg, const UdcMsgPingPong& body, UdcSignature signature, UdcMessageId messageId)
{
    msg.resize(UDC_MSG_PING_PONG_SIZE);
    udcGenerateHeader(msg, signature, messageId);

    uint8_t* ptr = msg.data() + UDC_MSG_HEADER_SIZE;

    memcpy(ptr, &body.clientId, sizeof(UdcEndPointId));
    ptr += sizeof(UdcEndPointId);

    uint32_t time = htonl(body.timeOnServer);
    memcpy(ptr, &time, sizeof(UdcMsgPingPong::timeOnServer));
}

bool udcReadMessage(const std::vector<uint8_t>& src, UdcMsgConnection& dst)
{
    if (src.size() != UDC_MSG_CONNECTION_SIZE)
    {
        return false;
    }

    const uint8_t* ptr = src.data() + UDC_MSG_HEADER_SIZE;

    memcpy(&dst.clientId, ptr, sizeof(UdcEndPointId));

    return true;
}

bool udcReadMessage(const std::vector<uint8_t>& src, UdcMsgExternal& dst)
{
    if (src.size() <= UDC_MSG_EXTERNAL_SIZE)
    {
        return false;
    }

    const uint8_t* ptr = src.data() + UDC_MSG_HEADER_SIZE;

    memcpy(&dst.clientId, ptr, sizeof(UdcEndPointId));

    dst.data = src.data() + UDC_MSG_EXTERNAL_SIZE;
    dst.size = src.size() - UDC_MSG_EXTERNAL_SIZE;

    return true;
}

bool udcReadMessage(const std::vector<uint8_t>& src, UdcMsgPingPong& dst)
{
    if (src.size() != UDC_MSG_PING_PONG_SIZE)
    {
        return false;
    }

    const uint8_t* ptr = src.data() + UDC_MSG_HEADER_SIZE;

    memcpy(&dst.clientId, ptr, sizeof(UdcEndPointId));
    ptr += sizeof(UdcEndPointId);

    uint32_t time;
    memcpy(&time, ptr, sizeof(UdcMsgPingPong::timeOnServer));
    dst.timeOnServer = htonl(time);

    return true;
}

// udp-connect
// Kyle J Burgess

#ifndef UDC_MESSAGE_H
#define UDC_MESSAGE_H

#include <cstdint>
#include <vector>

enum UdcMessageId : uint8_t
{
    UDC_MSG_CONNECTION_REQUEST,
    UDC_MSG_EXTERNAL,
};

struct UdcMsgHeader
{
    uint32_t signature;
    uint32_t timestamp;
    UdcMessageId messageId;
};

struct UdcMsgConnectionRequest
{
    uint16_t port;
};

void udcGenerateHeader(std::vector<uint8_t>& msg, const UdcMsgHeader& header);

void udcGenerateMsg(std::vector<uint8_t>& msg, const UdcMsgHeader& header, const UdcMsgConnectionRequest& body);

void udcReadMessage(uint32_t sig, const std::vector<uint8_t>& str, UdcMsgHeader& header, UdcMsgConnectionRequest& msg);

#endif

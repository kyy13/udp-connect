// udp-connect
// Kyle J Burgess

#include "UdcMessage.h"

#include <cassert>
#include <cstring>
#include <winsock2.h>

void udcGenerateHeader(std::vector<uint8_t>& msg, const UdcMsgHeader& header)
{
    assert(msg.size() >= 9);

    uint32_t sig = htonl(header.signature);
    uint32_t time = htonl(header.timestamp);

    memcpy(&msg[0], &sig, sizeof(sig));
    memcpy(&msg[4], &time, sizeof(time));
    memcpy(&msg[8], &header.messageId, sizeof(header.messageId));
}

void udcGenerateMsg(std::vector<uint8_t>& msg, const UdcMsgHeader& header, const UdcMsgConnectionRequest& body)
{

}

void udcReadMessage(uint32_t sig, const std::vector<uint8_t>& str, UdcMsgHeader& header, UdcMsgConnectionRequest& msg)
{

}

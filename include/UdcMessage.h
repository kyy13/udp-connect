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
    UDC_MSG_EXTERNAL,
};

struct UdcMsgConnection
{
    UdcDeviceId clientId;
    UdcDeviceId serverId;
};

constexpr size_t UDC_MSG_HEADER_SIZE =
    sizeof(UdcSignature) +
    sizeof(UdcMessageId);

constexpr size_t UDC_MSG_CONNECTION_SIZE =
    UDC_MSG_HEADER_SIZE +
    2 * sizeof(UdcDeviceId);

void udcGenerateHeader(std::vector<uint8_t>& msg, UdcSignature signature, UdcMessageId messageId);

[[nodiscard]]
bool udcReadHeader(const std::vector<uint8_t>& src, uint32_t signature, UdcMessageId& messageId);

void udcGenerateMessage(std::vector<uint8_t>& msg, const UdcMsgConnection& body, UdcSignature signature, UdcMessageId messageId);

[[nodiscard]]
bool udcReadMessage(const std::vector<uint8_t>& src, UdcMsgConnection& dst);

#endif

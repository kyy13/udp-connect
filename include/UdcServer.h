// udp-connect
// Kyle J Burgess

#ifndef UDC_SERVER_H
#define UDC_SERVER_H

#include "udp_connect.h"
#include "UdcSocketReceiver.h"
#include "UdcSocketSender.h"

#include <memory>
#include <vector>
#include <unordered_map>

struct UdcClientInfo
{
    UdcServiceId id;
    UdcStatus status;
    UdcSocketSender socket;
};

struct UdcServiceIdHasher
{
    uint32_t operator()(const UdcServiceId& clientId)
    {
        return
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[0]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[4]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[8]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[12]) ^
            *reinterpret_cast<const uint32_t*>(&clientId.bytes[16]);
    }
};

struct UdcServer
{
    UdcServiceId id;
    UdcSocketReceiver socket;
    std::unordered_map<UdcServiceId, std::unique_ptr<UdcClientInfo>, UdcServiceIdHasher> clients;
};

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDC_PACKET_LOGGER_H
#define UDC_PACKET_LOGGER_H

#include "UdcSocket.h"

#include <vector>
#include <string>
#include <fstream>

class UdcPacketLogger
{
public:

    UdcPacketLogger(const std::string& fileName);

    void logReceived(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size);

    void logReceived(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size);

    void logSent(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size);

    void logSent(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size);

protected:

    std::ofstream m_file;

};

#endif

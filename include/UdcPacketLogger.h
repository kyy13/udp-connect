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

    void logReceived(const UdcAddressIPv4& address, uint16_t port, const std::vector<uint8_t>& packetData);

    void logReceived(const UdcAddressIPv6& address, uint16_t port, const std::vector<uint8_t>& packetData);

    void logSent(const UdcAddressIPv4& address, uint16_t port, const std::vector<uint8_t>& packetData);

    void logSent(const UdcAddressIPv6& address, uint16_t port, const std::vector<uint8_t>& packetData);

protected:

    std::ofstream m_file;

};

#endif

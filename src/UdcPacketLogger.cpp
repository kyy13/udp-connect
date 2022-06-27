// udp-connect
// Kyle J Burgess

#include "UdcPacketLogger.h"

#include <stdexcept>
#include <iomanip>

UdcPacketLogger::UdcPacketLogger(const std::string& fileName)
    : m_file(fileName, std::ios::trunc)
{
    if (m_file.is_open())
    {
        m_file << "Log Start\n";
    }
    else
    {
        throw std::runtime_error("Could not open log file!");
    }
}

void UdcPacketLogger::logReceived(const UdcAddressIPv4& address, uint16_t port, const std::vector<uint8_t>& packetData)
{
    m_file << "RECV IPV4 ";

    for (uint8_t byte : address.octets)
    {
        m_file
            << std::dec
            << static_cast<uint32_t>(byte)
            << '.';
    }

    m_file << ':' << std::dec << port << ' ';

    for (uint8_t byte : packetData)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << std::right
            << static_cast<uint32_t>(byte)
            << ' ';
    }

    m_file << '\n';
}

void UdcPacketLogger::logReceived(const UdcAddressIPv6& address, uint16_t port, const std::vector<uint8_t>& packetData)
{
    m_file << "RECV IPV6 ";

    for (uint16_t segment : address.segments)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << std::right
            << segment
            << ':';
    }

    m_file << ':' << std::dec << port << ' ';

    for (uint8_t byte : packetData)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << std::right
            << static_cast<uint32_t>(byte)
            << ' ';
    }

    m_file << '\n';
}

void UdcPacketLogger::logSent(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size)
{
    m_file << "SEND IPV4 ";

    for (uint8_t byte : address.octets)
    {
        m_file
            << std::dec
            << static_cast<uint32_t>(byte)
            << '.';
    }

    m_file << ':' << std::dec << port << ' ';

    for (uint32_t i = 0; i != size; ++i)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << std::right
            << static_cast<uint32_t>(data[i])
            << ' ';
    }

    m_file << '\n';
}

void UdcPacketLogger::logSent(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size)
{
    m_file << "SEND IPV6 ";

    for (uint16_t segment : address.segments)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << std::right
            << segment
            << '.';
    }

    m_file << ':' << std::dec << port << ' ';

    for (uint32_t i = 0; i != size; ++i)
    {
        m_file
            << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << std::right
            << static_cast<uint32_t>(data[i])
            << ' ';
    }

    m_file << '\n';
}

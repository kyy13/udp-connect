// udp-connect
// Kyle J Burgess

#include "UdcSocketMux.h"

UdcSocketMux::UdcSocketMux() = default;

UdcSocketMux::UdcSocketMux(const std::string& logFileName)
{
    try
    {
        m_logger = std::make_unique<UdcPacketLogger>(logFileName);
    }
    catch(...)
    {}
}

bool UdcSocketMux::bind(uint16_t portIPv6, uint16_t portIPv4)
{
    bool result = false;

    m_socketIPv6.disconnect();
    m_socketIPv4.disconnect();

    if (m_socketIPv6.localBindIPv6(portIPv6, false))
    {
        result = true;
    }

    if (m_socketIPv4.localBindIPv4(portIPv4))
    {
        result = true;
    }

    return result;
}

bool UdcSocketMux::send(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size) const
{
    // Not connected
    if (!m_socketIPv4.isConnected())
    {
        return false;
    }

    bool result = m_socketIPv4.sendIPv4(address, port, data, size);

    // Log if necessary
    if (m_logger && result)
    {
        m_logger->logSent(address, port, data, size);
    }

    return result;
}

bool UdcSocketMux::send(const UdcAddressIPv6& address, uint16_t port, const uint8_t* data, uint32_t size) const
{
    // Not connected
    if (!m_socketIPv6.isConnected())
    {
        return false;
    }

    bool result = m_socketIPv6.sendIPv6(address, port, data, size);

    // Log if necessary
    if (m_logger && result)
    {
        m_logger->logSent(address, port, data, size);
    }

    return result;
}

bool UdcSocketMux::receive(UdcAddressIPv4& address, uint16_t& port, std::vector<uint8_t>& data)
{
    // Not connected
    if (!m_socketIPv4.isConnected())
    {
        return false;
    }

    bool result = m_socketIPv4.receiveIPv4(address, port, data) == 1;

    // Log if necessary
    if (m_logger && result)
    {
        m_logger->logReceived(address, port, data);
    }

    return result;
}

bool UdcSocketMux::receive(UdcAddressIPv6& address, uint16_t& port, std::vector<uint8_t>& data)
{
    // Not connected
    if (!m_socketIPv6.isConnected())
    {
        return false;
    }

    bool result = m_socketIPv6.receiveIPv6(address, port, data) == 1;

    // Log if necessary
    if (m_logger && result)
    {
        m_logger->logReceived(address, port, data);
    }

    return result;
}

void UdcSocketMux::disconnect()
{
    m_socketIPv4.disconnect();
    m_socketIPv6.disconnect();
}

bool UdcSocketMux::isConnected() const
{
    return m_socketIPv4.isConnected() || m_socketIPv6.isConnected();
}

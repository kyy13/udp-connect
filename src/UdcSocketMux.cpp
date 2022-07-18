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

bool UdcSocketMux::tryBindIPv4(uint16_t port)
{
    UdcSocket socket;

    if (socket.localBindIPv4(port))
    {
        m_socketIPv4.push_back(std::move(socket));
        return true;
    }

    return false;
}

bool UdcSocketMux::tryBindIPv6(uint16_t port)
{
    UdcSocket socket;

    if (socket.localBindIPv6(port))
    {
        m_socketIPv6.push_back(std::move(socket));
        return true;
    }

    return false;
}

bool UdcSocketMux::send(const UdcAddressMux& address, const uint8_t* data, uint32_t size) const
{
    if (address.family == UDC_IPV6)
    {
        return send(address.address.ipv6, address.port, data, size);
    }

    return send(address.address.ipv4, address.port, data, size);
}

bool UdcSocketMux::send(const UdcAddressIPv4& address, uint16_t port, const uint8_t* data, uint32_t size) const
{
    // Not connected
    if (m_socketIPv4.empty())
    {
        return false;
    }

    bool result = m_socketIPv4.front().sendIPv4(address, port, data, size);

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
    if (m_socketIPv6.empty())
    {
        return false;
    }

    bool result = m_socketIPv6.front().sendIPv6(address, port, data, size);

    // Log if necessary
    if (m_logger && result)
    {
        m_logger->logSent(address, port, data, size);
    }

    return result;
}

bool UdcSocketMux::receive(UdcAddressMux& address, uint8_t* buffer, uint32_t& size)
{
    if (receive(address.address.ipv6, address.port, buffer, size))
    {
        address.family = UDC_IPV6;
        return true;
    }

    if (receive(address.address.ipv4, address.port, buffer, size))
    {
        address.family = UDC_IPV4;
        return true;
    }

    return false;
}

bool UdcSocketMux::receive(UdcAddressIPv4& address, uint16_t& port, uint8_t* buffer, uint32_t& size)
{
    for (auto& socket : m_socketIPv4)
    {
        if (socket.receiveIPv4(address, port, buffer, size) == 1)
        {
            if (m_logger)
            {
                m_logger->logReceived(address, port, buffer, size);
            }

            return true;
        }
    }

    return false;
}

bool UdcSocketMux::receive(UdcAddressIPv6& address, uint16_t& port, uint8_t* buffer, uint32_t& size)
{
    for (auto& socket : m_socketIPv6)
    {
        if (socket.receiveIPv6(address, port, buffer, size) == 1)
        {
            if (m_logger)
            {
                m_logger->logReceived(address, port, buffer, size);
            }

            return true;
        }
    }

    return false;
}

void UdcSocketMux::disconnect()
{
    m_socketIPv4.clear();
    m_socketIPv6.clear();
}

bool UdcSocketMux::isConnected() const
{
    return !(m_socketIPv4.empty() && m_socketIPv6.empty());
}

// udp-connect
// Kyle J Burgess

#include "UdcDualSocket.h"

bool UdcDualSocket::bind(uint16_t portIPv6, uint16_t portIPv4)
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

bool UdcDualSocket::send(const UdcAddressIPv4& address, uint16_t port, const std::vector<uint8_t>& data) const
{
    // Not connected
    if (!m_socketIPv4.isConnected())
    {
        return false;
    }

    return m_socketIPv4.sendIPv4(address, port, data);
}

bool UdcDualSocket::send(const UdcAddressIPv6& address, uint16_t port, const std::vector<uint8_t>& data) const
{
    // Not connected
    if (!m_socketIPv6.isConnected())
    {
        return false;
    }

    return m_socketIPv6.sendIPv6(address, port, data);
}

bool UdcDualSocket::receive(UdcAddressIPv4& address, uint16_t& port, std::vector<uint8_t>& data)
{
    // Not connected
    if (!m_socketIPv4.isConnected())
    {
        return false;
    }

    return m_socketIPv4.receiveIPv4(address, port, data) == 1;
}

bool UdcDualSocket::receive(UdcAddressIPv6& address, uint16_t& port, std::vector<uint8_t>& data)
{
    // Not connected
    if (!m_socketIPv6.isConnected())
    {
        return false;
    }

    return m_socketIPv6.receiveIPv6(address, port, data) == 1;
}

void UdcDualSocket::disconnect()
{
    m_socketIPv4.disconnect();
    m_socketIPv6.disconnect();
}

bool UdcDualSocket::isConnected() const
{
    return m_socketIPv4.isConnected() || m_socketIPv6.isConnected();
}

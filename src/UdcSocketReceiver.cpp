// udp-connect
// Kyle J Burgess

#include "UdcSocketReceiver.h"

bool UdcSocketReceiver::bind(uint16_t primaryPortIPv6, uint16_t backupPortIPv4)
{
    // Disconnected if already connected
    if (m_primarySocketIPv6.isConnected())
    {
        m_primarySocketIPv6.disconnect();
        m_backupSocketIPv4.disconnect();
    }

    // Attempt to bind IPv6 port with dual stack IPv4 support
    if (m_primarySocketIPv6.localBindIPv6(primaryPortIPv6, true))
    {
        return true;
    }

    // Attempt to bind IPv6 on primary socket and IPv4 on secondary socket
    if (m_primarySocketIPv6.localBindIPv6(primaryPortIPv6, false) &&
        m_backupSocketIPv4.localBindIPv4(backupPortIPv4))
    {
        return true;
    }

    // Failed to bind
    return false;
}

bool UdcSocketReceiver::receive(
    UdcAddressFamily& addressFamily,
    UdcAddress& address,
    std::vector<uint8_t>& data)
{
    // Not connected
    if (!m_primarySocketIPv6.isConnected())
    {
        return false;
    }

    int32_t result;

    // IPv6 connected with dual stack IPv4 support
    if (!m_backupSocketIPv4.isConnected())
    {
        addressFamily = UDC_IPV6;
        result = m_primarySocketIPv6.receiveIPv6(address.ipv6, data);
        return (result == 1);
    }

    // IPv6 and IPv4 connected separately
    addressFamily = UDC_IPV6;
    result = m_primarySocketIPv6.receiveIPv6(address.ipv6, data);

    if (result == 1)
    {
        return true;
    }

    addressFamily = UDC_IPV4;
    result = m_backupSocketIPv4.receiveIPv4(address.ipv4, data);

    return (result == 1);
}

void UdcSocketReceiver::disconnect()
{
    // Disconnected if already connected
    if (m_primarySocketIPv6.isConnected())
    {
        m_primarySocketIPv6.disconnect();
        m_backupSocketIPv4.disconnect();
    }
}

bool UdcSocketReceiver::isConnected() const
{
    return m_primarySocketIPv6.isConnected();
}

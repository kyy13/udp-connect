// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"

bool UdcSocketSender::connect(const UdcAddressIPv4& remoteIp, uint16_t remotePort)
{
    m_socket.disconnect();
    return m_socket.remoteConnect(remoteIp, remotePort);
}

bool UdcSocketSender::connect(const UdcAddressIPv6& remoteIp, uint16_t remotePort)
{
    m_socket.disconnect();
    return m_socket.remoteConnect(remoteIp, remotePort);
}

bool UdcSocketSender::send(const std::vector<uint8_t>& data) const
{
    return m_socket.send(data);
}

void UdcSocketSender::disconnect()
{
    m_socket.disconnect();
}

bool UdcSocketSender::isConnected() const
{
    return m_socket.isConnected();
}

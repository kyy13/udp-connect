// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"

bool UdcSocketSender::connect(const std::string& nodeName, const std::string& serviceName)
{
    m_socket.disconnect();
    return m_socket.remoteConnect(nodeName, serviceName);
}

bool UdcSocketSender::connectIPv6(const UdcAddressIPv6& remoteIp, uint16_t remotePort)
{
    m_socket.disconnect();
    return m_socket.remoteConnectIPv6(remoteIp, remotePort);
}

bool UdcSocketSender::connectIPv4(const UdcAddressIPv4& remoteIp, uint16_t remotePort)
{
    m_socket.disconnect();
    return m_socket.remoteConnectIPv4(remoteIp, remotePort);
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

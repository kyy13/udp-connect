// udp-connect
// Kyle J Burgess

#ifndef UDC_CONNECTION_H
#define UDC_CONNECTION_H

#include "UdcServer.h"
#include "UdcDeviceId.h"
#include "UdcMessage.h"
#include "UdcEvent.h"

// UDC_MSG_CONNECTION_REQUEST
void trySendConnectionRequest(UdcServer* server, UdcClientInfo& client)
{
    constexpr auto clientInternalSendTime = std::chrono::milliseconds {200};

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    if ((time - client.lastSendTime) >= clientInternalSendTime)
    {
        // Send connection request
        const UdcMsgConnection msgConnectionRequest =
            {
                .clientId = client.id,
                .serverId = server->id,
            };

        udcGenerateMessage(
            server->messageBuffer,
            msgConnectionRequest,
            server->signature,
            UDC_MSG_CONNECTION_REQUEST);

        if (client.addressFamily == UDC_IPV6)
        {
            server->socket.send(client.addressIPv6, client.port, server->messageBuffer);
        }
        else if (client.addressFamily == UDC_IPV4)
        {
            server->socket.send(client.addressIPv4, client.port, server->messageBuffer);
        }

        client.lastSendTime = time;
    }
}

// UDC_MSG_CONNECTION_REQUEST
template<class T>
void tryReadConnectionRequest(UdcServer* server, const T& address, uint16_t port)
{
    UdcMsgConnection msg;

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return;
    }

    // If this server doesn't know its ID, then grab it from the connection request
    if (isNullDeviceId(server->id))
    {
        server->id = msg.clientId;
    }
    else // Otherwise, send this server's ID
    {
        msg.clientId = server->id;
    }

    // If the client doesn't know its ID, then generate one for it
    if (isNullDeviceId(msg.serverId))
    {
        msg.serverId = newDeviceId(address, port);
    }

    // Generate a handshake response
    udcGenerateMessage(
        server->messageBuffer,
        msg,
        server->signature,
        UDC_MSG_CONNECTION_HANDSHAKE);

    // Send handshake
    server->socket.send(address, port, server->messageBuffer);
}

// UDC_MSG_CONNECTION_HANDSHAKE
template<class T>
bool tryReadConnectionHandshake(UdcServer* server, const T& address, UdcEvent& event)
{
    UdcMsgConnection msg;

    // If there are no pending clients,
    // then ignore
    if (server->pendingClients.empty())
    {
        return false;
    }

    // Parse the message
    if (!udcReadMessage(server->messageBuffer, msg))
    {
        return false;
    }

    // Check for matching server IDs and client IDs
    if (cmpDeviceId(server->id, msg.serverId) &&
        cmpDeviceId(server->pendingClients.front()->id, msg.clientId))
    {
        auto& client = server->pendingClients.front();

        event.eventType = UDC_EVENT_CONNECTION_SUCCESS;
        event.endPointId = msg.clientId;
        event.nodeName = client->nodeName.c_str();
        event.serviceName = client->serviceName.c_str();

        server->clients[msg.clientId] = std::move(client);
        server->pendingClients.pop();

        return true;
    }

    // If this server doesn't know its ID, then grab it from the handshake
    if (isNullDeviceId(server->id))
    {
        server->id = msg.serverId;
    }
}

#endif

// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    // A local server
    struct UdcServer;

    // A unique identifier for a service
    struct UdcDeviceId
    {
        uint8_t bytes[20];
    };

    using UdcSignature = uint32_t;

    enum UdcStatus : uint8_t
    {
        UDC_NOT_CONNECTED,
        UDC_CONNECTING,
        UDC_RECONNECTING,
        UDC_CONNECTED,
    };

    enum UdcReliability : uint8_t
    {
        UDC_UNRELIABLE,
        UDC_RECEIVED_AT_LEAST_ONCE,
        UDC_RECEIVED_ONCE,
        UDC_RECEIVED_IN_ORDER_ONCE,
    };

    enum UdcResult : uint32_t
    {
        UDC_SUCCESS,
        UDC_BIND_SOCKET_ERROR,
        UDC_ALREADY_CONNECTED,
    };

    // Creates a local server responsible for reading and acknowledging
    // messages from remote clients
    // returns nullptr if it fails to connect
    UdcServer* udcCreateServer(
        uint32_t signature,
        uint16_t portIPv4,
        uint16_t portIPv6,
        const char* logFileName);

    // Stops and deletes a server
    void udcDeleteServer(UdcServer* server);

    // Try to connect to a client from a server
    // timeout is time in milliseconds
    bool udcTryConnect(
        UdcServer* server,
        const char* ipString,
        const char* portString,
        uint32_t timeout);

    uint32_t udcGetConnectionCount(UdcServer* server);

//    // Disconnect from a client
//    // Removing all pending messages (sent and received)
//    void udcDisconnect(UdcServer* server, const UdcDeviceId& clientId);

    // Update and read messages from the server
    // returns a pointer to a message buffer and the size of the message
    // returns nullptr when there are no messages left to read
    // Call function every frame until result is nullptr
    uint8_t* udcReceive(UdcServer* server, UdcDeviceId& clientId, uint32_t& size);

//
//    // Send a message to the client
//    UdcResult udcWriteClient(UdcClient* client, const char* message, UdcReliability reliability);
//
//    // Get the status of a client
//    UdcStatus udcGetClientStatus(UdcClient* client);
//
//    // Get the ping (in ms) of the client
//    uint32_t udcGetClientPing(UdcClient* client);
}

#endif

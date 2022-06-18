// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    // A local server
    struct UdcServer;

    // A connection to a remove client
    struct UdcClient;

    // IP version
    enum UdcAddressFamily : uint8_t
    {
        UDC_IPV4,
        UDC_IPV6,
    };

    // IPv4 Address
    // Stored as 4 octets in the format [0].[1].[2].[3]
    struct UdcAddressIPv4
    {
        uint8_t octets[4];
    };

    // IPv6 Address
    // Stored as 8 segments in the format [0]:[1]:[2]:[3]:[4]:[5]:[6]:[7]
    // where each segment is in network byte-order (Big Endian)
    struct UdcAddressIPv6
    {
        uint16_t segments[8];
    };

    enum UdcStatus : uint8_t
    {
        UDC_NOT_CONNECTED,
        UDC_CONNECTION_FAILED,
        UDC_CONNECTION_LOST,
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
    UdcServer* udcCreateServer();

    // Call udcStopServer() and delete the server
    void udcDeleteServer(UdcServer* server);

    // Starts a server on a port allowing clients to connect
    UdcResult udcStartServer(int port);

    // Stops a server and disconnects clients
    // clears pending incoming/outgoing messages
    void udcStopServer(UdcServer* server);

    // Creates a handle to a client that can be connected to
    UdcClient* udcCreateClient();

    // Calls udcDisconnect() and deletes the handle to client
    void udcDeleteClient(UdcClient* client);

    // Try to connect to a client from a server
    // Call if state is UDC_NOT_CONNECTED or UDC_CONNECTION_FAILED
    UdcResult udcTryConnect(UdcServer* server, UdcClient* client, const char* ip, int port, float timeout, int attempts);

    // Attempt to reconnect after disconnecting
    // Call if state is UDC_CONNECTION_LOST
    // If the ip, port, or server needs to be changed, then disconnect first and use udcTryConnect
    UdcResult udcTryReconnect(UdcClient* client, float timeout, int attempts);

    // Disconnect from a client
    // Removing all pending messages (sent and received)
    void udcDisconnect(UdcClient* client);

    // Read messages from the server
    // returns true until all messages have been read
    bool udcReadServer(UdcClient*& client, char* message, uint32_t& size);

    // Send a message to the client
    UdcResult udcWriteClient(UdcClient* client, const char* message, UdcReliability reliability);

    // Get the status of a client
    UdcStatus udcGetClientStatus(UdcClient* client);

    // Get the ping (in ms) of the client
    uint32_t udcGetClientPing(UdcClient* client);
}

#endif

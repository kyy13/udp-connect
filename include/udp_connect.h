// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    struct UdcClient;

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

    // Starts the receiving UDP server
    UdcResult udcStartServer(int port);

    // Stops the receiving UDP server
    void udcStopServer();

    // Creates a client that can be connected to
    UdcClient* udcCreateClient();

    // Destroys a client and the connection to it
    void udcDestroyClient(UdcClient* client);

    // Try to connect to a client from a server
    // Call if state is UDC_NOT_CONNECTED or UDC_CONNECTION_FAILED
    UdcResult udcTryConnect(UdcClient* client, const char* ip, int port, float timeout, int attempts);

    // Attempt to reconnect after disconnecting
    // Call if state is UDC_CONNECTION_LOST
    // If the ip/port needs to be changed, then disconnect first and use udcTryConnect
    UdcResult udcTryReconnect(UdcClient* client, float timeout, int attempts);

    // Disconnect from a client
    // Removing all pending messages (sent and received)
    void udcDisconnect(UdcClient* client);

    // Read unassigned messages from the server
    // returns true until all messages have been read
    bool udcReadServer(UdcClient* client, char* message, uint32_t& size);

    // Read messages from a client
    // returns true until all messages have been read
    bool udcReadClient(UdcClient* client, char* message, uint32_t& size);

    // Send a message to the client
    UdcResult udcWriteClient(UdcClient* client, const char* message, UdcReliability reliability);

    // Get the status of a client
    UdcStatus udcGetClientStatus(UdcClient* client);

    // Get the ping (in ms) of the client
    uint32_t udcGetClientPing(UdcClient* client);
}

#endif

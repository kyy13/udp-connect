// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    // IP version
    enum IpVersion : uint8_t
    {
        IP_V4,
        IP_V6,
    };

    // IP address union for IPv4 and IPv6
    // IPv4 readable format by byte number is
    //  [0].[1].[2].[3]
    // IPv6 readable format by byte number is
    //  [0][1]:[2][3]:[4][5]:[6][7]:[8][9]:[10][11]:[12][13]:[14][15]
    //  NOTE: Byte pairs (e.g. [0][1]) in Big Endian byte order
    union IpAddress
    {
        uint8_t ip_v4[4];
        uint8_t ip_v6[16];
    };

    // A local server
    struct UdcServer;

    // A connection to a remove client
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

    // Convert an IP string to an IpVersion and IpAddress
    // returns true on success
    bool udcConvertStringToIp(const char* str, IpVersion* version, IpAddress* address);

    // Convert an IP version and address to an IP string
    // returns the length of the resulting string
    // If the value of str is nullptr, then it will still return the resulting length,
    // but it will not write to str
    uint32_t udcConvertIpToString(IpVersion version, IpAddress address, char* str);
}

#endif

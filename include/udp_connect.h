// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    // A local server
    struct UdcServer;

    // An event
    struct UdcEvent;

    // Types of events
    enum UdcEventType : uint32_t
    {
        // A connection attempt has succeeded
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_SUCCESS = 0u,

        // A connection attempt has timed out (failed)
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_TIMEOUT = 1u,

        // A connection was abnormally lost, the endpoint connection will continue trying to connect
        // and will stop queueing unreliable messages to send until a UDC_EVENT_CONNECTION_REGAINED event
        // happens, or udcDisconnect() is called.
        UDC_EVENT_CONNECTION_LOST = 2u,

        // A connection that was lost has been regained.
        UDC_EVENT_CONNECTION_REGAINED = 3u,

        // A message has been received.
        UDC_EVENT_RECEIVE_MESSAGE_IPV4 = 4u,

        // A message has been received.
        UDC_EVENT_RECEIVE_MESSAGE_IPV6 = 5u,
    };

    enum UdcReliability
    {
        UDC_UNRELIABLE_PACKET,
        UDC_RELIABLE_PACKET,
    };

    // A locally unique identifier for a node
    using UdcEndPointId = uint32_t;

    // Packet signature
    using UdcSignature = uint32_t;

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

    // Creates a local server responsible for reading and acknowledging
    // messages from remote clients
    // returns nullptr if it fails to connect
    UdcServer* udcCreateServer(
        UdcSignature signature,
        uint16_t portIPv6,
        uint16_t portIPv4,
        uint8_t* buffer,
        uint32_t size,
        const char* logFileName);

    // Stops and deletes a server
    void udcDeleteServer(UdcServer* server);

    // Try to parse a node and service string into an IP and port number
    bool udcTryParseAddressIPv4(
        const char* nodeName,
        const char* serviceName,
        UdcAddressIPv4& address,
        uint16_t& port);

    bool udcTryParseAddressIPv6(
        const char* nodeName,
        const char* serviceName,
        UdcAddressIPv6& address,
        uint16_t& port);

    // Try to connect to a client from a server
    // Timeout (milliseconds) represents the amount of time a connection has to establish itself before
    // UDC_EVENT_CONNECTION_TIMEOUT is called and the connection is aborted.
    // Timeout also represents the amount of time that the server can receive no messages (including ping tests)
    // from the client before calling UDC_EVENT_CONNECTION_LOST and trying to reestablish a connection.
    bool udcTryConnect(
        UdcServer* server,
        const char* nodeName,
        const char* serviceName,
        uint32_t timeout,
        UdcEndPointId& endPointId);

    bool udcTryConnectIPv4(
        UdcServer* server,
        const UdcAddressIPv4& ip,
        uint16_t port,
        uint32_t timeout,
        UdcEndPointId& endPointId);

    bool udcTryConnectIPv6(
        UdcServer* server,
        const UdcAddressIPv6& ip,
        uint16_t port,
        uint32_t timeout,
        UdcEndPointId& endPointId);

    void udcSendMessage(
        UdcServer* server,
        UdcEndPointId endPointId,
        const uint8_t* data,
        uint32_t size,
        UdcReliability reliability);

    // Main update loop
    // process events until nullptr is returned
    UdcEvent* udcProcessEvents(UdcServer* server);

    // Get the type of event that was returned by udcProcessEvents()
    UdcEventType udcGetEventType(const UdcEvent* event);

    // Get results of a connection event
    bool udcGetResultConnectionEvent(const UdcEvent* event, UdcEndPointId& endPointId);

    // Get results of an external message event
    bool udcGetResultExternalIPv4Event(const UdcEvent* event, UdcEndPointId& endPointId, UdcAddressIPv4& address, uint16_t& port, uint8_t* buffer, uint32_t& size);

    // Get results of an external message event
    bool udcGetResultExternalIPv6Event(const UdcEvent* event, UdcEndPointId& endPointId, UdcAddressIPv6& address, uint16_t& port, uint8_t* buffer, uint32_t& size);
}

#endif

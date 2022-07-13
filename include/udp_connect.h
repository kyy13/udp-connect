// udp-connect
// Kyle J Burgess

#ifndef UDP_CONNECT_H
#define UDP_CONNECT_H

#include <cstdint>

extern "C"
{
    // A local server
    struct                  UdcServer;

    // An event
    struct                  UdcEvent;

    // Types of events
    enum                    UdcEventType : uint32_t
    {
        // A connection attempt has succeeded
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_SUCCESS   = 0u,

        // A connection attempt has timed out (failed)
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_TIMEOUT   = 1u,

        // A connection was abnormally lost, the endpoint connection will continue trying to connect
        // and will stop queueing unreliable messages to send until a UDC_EVENT_CONNECTION_REGAINED event
        // happens, or udcDisconnect() is called.
        UDC_EVENT_CONNECTION_LOST      = 2u,

        // A connection that was lost has been regained.
        UDC_EVENT_CONNECTION_REGAINED  = 3u,

        // A message has been received.
        UDC_EVENT_RECEIVE_MESSAGE_IPV4 = 4u,

        // A message has been received.
        UDC_EVENT_RECEIVE_MESSAGE_IPV6 = 5u,
    };

    // Types of messages
    enum                    UdcMessageType : uint32_t
    {
        // Unreliable packets are basic UDP packets
        // there is no guarantee that they arrive
        UDC_UNRELIABLE_MESSAGE         = 0u,

        // Reliable packets are guaranteed to arrive in order
        // and arrive exactly once. If connection is lost, then the reliable
        // messages will wait to be sent until connection is regained. If the
        // connection is closed with udcDisconnect, then pending reliable messages
        // are cleared.
        UDC_RELIABLE_MESSAGE           = 1u,
    };

    // A locally unique identifier for a node
    typedef uint32_t        UdcEndPointId;

    // Message signature
    struct                  UdcSignature
    {
        uint8_t bytes[4];
    };

    // IPv4 Address
    // Stored as 4 octets in the format [0].[1].[2].[3]
    struct                  UdcAddressIPv4
    {
        uint8_t octets[4];
    };

    // IPv6 Address
    // Stored as 8 segments in the format [0]:[1]:[2]:[3]:[4]:[5]:[6]:[7]
    // where each segment is in network byte-order (Big Endian)
    struct                  UdcAddressIPv6
    {
        uint16_t segments[8];
    };

    // Returns the minimum size of the message buffer (in bytes)
    // that needs to be given to udcCreateServer()
    uint32_t        __cdecl udcGetMinimumBufferSize();

    // Creates a local server responsible for reading and acknowledging messages from remote clients
    // returns nullptr if it fails to connect
    UdcServer*      __cdecl udcCreateServer(
        UdcSignature           signature,    // A custom signature that recognizes packets as valid
                                             // other servers need to have the same value in order to send/receive.
        uint16_t               portIPv6,     // The port the server listens on for IPv6 connections and messages
        uint16_t               portIPv4,     // The port the server listens on for IPv4 connections and messages
        uint8_t*               buffer,       // The handle to a buffer that can be used for storing sent/received messages
                                             // the buffer must be at least udcGetMinimumBufferSize() size in bytes
        uint32_t               size,         // The size of buffer (in bytes)
        const char*            logFileName); // Nullptr for no debugging, or the name of a message log file for debugging

    // Stops and deletes a server
    void            __cdecl udcDeleteServer(
        UdcServer*             server);      // Delete a server and frees any memory associated with the server

    // Try to parse a node and service null-terminated string into an IPv4 address and port number
    // returns true on success
    bool            __cdecl udcTryParseAddressIPv4(
        const char*            nodeName,     // A node name or ip address
        const char*            serviceName,  // A service name or port
        UdcAddressIPv4&        address,      // The returned IPv4 address
        uint16_t&              port);        // The returned port number

    // Try to parse a node and service null-terminated string into an IPv6 address and port number
    bool            __cdecl udcTryParseAddressIPv6(
        const char*            nodeName,     // A node name or ip address
        const char*            serviceName,  // A service name or port
        UdcAddressIPv6&        address,      // The returned IPv6 address
        uint16_t&              port);        // The returned port number

    // Try to connect to a client from a server
    // Timeout (milliseconds) represents the amount of time a connection has to establish itself before
    // UDC_EVENT_CONNECTION_TIMEOUT is called and the connection is aborted.
    // Timeout also represents the amount of time that the server can receive no messages (including ping tests)
    // from the client before calling UDC_EVENT_CONNECTION_LOST and trying to reestablish a connection.
    // A connection lost event does not clear the reliable message queue.
    // Returns false immediately if it fails to connect to port
    bool            __cdecl udcTryConnect(
        UdcServer*             server,       // The local server to connect from
        const char*            nodeName,     // The null-terminated node name or ip address to connect to
        const char*            serviceName,  // The null-terminated service name or port to connect to
        uint32_t               timeout,      // The timeout (ms) for the connection
        UdcEndPointId&         endPointId);  // The returned endpoint ID matched with udcGetResultConnectionEvent
                                             // for monitoring UDC_EVENT_CONNECTION_SUCCESS or UDC_EVENT_CONNECTION_TIMEOUT

    // Try to connect to a client from a server using an IPv4 address
    // See udcTryConnect for details
    bool            __cdecl udcTryConnectIPv4(
        UdcServer*             server,       // The local server to connect from
        const UdcAddressIPv4&  ip,           // The IPv4 address to connect to
        uint16_t               port,         // The port number to connect to
        uint32_t               timeout,      // The timeout (ms) for the connection
        UdcEndPointId&         endPointId);  // The returned endpoint ID matched with udcGetResultConnectionEvent
                                             // for monitoring UDC_EVENT_CONNECTION_SUCCESS or UDC_EVENT_CONNECTION_TIMEOUT

    // Try to connect to a client from a server using an IPv6 address
    // See udcTryConnect for details
    bool            __cdecl udcTryConnectIPv6(
        UdcServer*             server,       // The local server to connect from
        const UdcAddressIPv6&  ip,           // The IPv4 address to connect to
        uint16_t               port,         // The port number to connect to
        uint32_t               timeout,      // The timeout (ms) for the connection
        UdcEndPointId&         endPointId);  // The returned endpoint ID matched with udcGetResultConnectionEvent
                                             // for monitoring UDC_EVENT_CONNECTION_SUCCESS or UDC_EVENT_CONNECTION_TIMEOUT

    // Manually disconnect from an endpoint and clear the reliable message queue
    void            __cdecl udcDisconnect(
        UdcServer*             server,
        UdcEndPointId          endPointId);

    // Send a message
    // returns false if the provided buffer is too small
    // or if the endPointId doesn't exist
    bool            __cdecl udcSendMessage(
        UdcServer*             server,       // The local server to send from
        UdcEndPointId          endPointId,   // The endpoint ID of the client (connected)
        const uint8_t*         data,         // The message
        uint32_t               size,         // The size of the message in bytes
        UdcMessageType         reliability); // The type of message

    // Main update loop
    // every frame, call udcProcessEvents() until nullptr is returned
    const UdcEvent* __cdecl udcProcessEvents(
        UdcServer*             server);      // The local server

    // Get the type of event that was returned by udcProcessEvents()
    UdcEventType    __cdecl udcGetEventType(
        const UdcEvent*        event);       // The event

    // Processes the following connection events and returns
    // the correlated endpoint
    // UDC_EVENT_CONNECTION_SUCCESS
    // UDC_EVENT_CONNECTION_TIMEOUT
    // UDC_EVENT_CONNECTION_LOST
    // UDC_EVENT_CONNECTION_REGAINED
    bool            __cdecl udcGetResultConnectionEvent(
        const UdcEvent*        event,        // The event
        UdcEndPointId&         endPointId);  // The correlating endpoint

    // Get results of an external message event (IPv4)
    // UDC_EVENT_RECEIVE_MESSAGE_IPV4
    bool            __cdecl udcGetResultExternalIPv4Event(
        const UdcEvent*        event,
        UdcAddressIPv4&        address,
        uint16_t&              port,
        uint32_t&              msgIndex,
        uint32_t&              msgSize);

    // Get results of an external message event (IPv6)
    // UDC_EVENT_RECEIVE_MESSAGE_IPV6
    bool            __cdecl udcGetResultExternalIPv6Event(
        const UdcEvent*        event,
        UdcAddressIPv6&        address,
        uint16_t&              port,
        uint32_t&              msgIndex,
        uint32_t&              msgSize);
}

#endif

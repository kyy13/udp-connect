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
    enum UdcEventType
    {
        // A connection attempt has succeeded
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_SUCCESS,

        // A connection attempt has timed out (failed)
        // *these events will happen in the order of udcTryConnect() calls
        UDC_EVENT_CONNECTION_TIMEOUT,

        // A connection was abnormally lost, the endpoint connection will continue trying to connect
        // and will stop queueing unreliable messages to send until a UDC_EVENT_CONNECTION_REGAINED event
        // happens, or udcDisconnect() is called.
        UDC_EVENT_CONNECTION_LOST,

        // A connection that was lost has been regained.
        UDC_EVENT_CONNECTION_REGAINED,

        // A message has been received.
        UDC_EVENT_RECEIVE_MESSAGE,
    };

    // A globally unique identifier for a service
    struct UdcEndPointId
    {
        uint8_t bytes[20];
    };

    // Packet signature
    using UdcSignature = uint32_t;

    // Creates a local server responsible for reading and acknowledging
    // messages from remote clients
    // returns nullptr if it fails to connect
    UdcServer* udcCreateServer(
        uint32_t signature,
        uint16_t portIPv6,
        uint16_t portIPv4,
        const char* logFileName);

    // Stops and deletes a server
    void udcDeleteServer(UdcServer* server);

    // Try to connect to a client from a server
    // Timeout (milliseconds) represents the amount of time a connection has to establish itself before
    // UDC_EVENT_CONNECTION_TIMEOUT is called and the connection is aborted.
    // Timeout also represents the amount of time that the server can receive no messages (including ping tests)
    // from the client before calling UDC_EVENT_CONNECTION_LOST and trying to reestablish a connection.
    bool udcTryConnect(
        UdcServer* server,
        const char* nodeName,
        const char* serviceName,
        uint32_t timeout);

    // Main update loop
    // process events until nullptr is returned
    UdcEvent* udcProcessEvents(UdcServer* server);

    // Get the type of event that was returned by udcProcessEvents()
    UdcEventType udcGetEventType(const UdcEvent* event);
}

#endif

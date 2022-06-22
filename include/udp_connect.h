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
        UDC_EVENT_CONNECTION_SUCCESS,
        UDC_EVENT_CONNECTION_TIMEOUT,
        UDC_EVENT_CONNECTION_LOST,
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
        uint16_t portIPv4,
        uint16_t portIPv6,
        const char* logFileName);

    // Stops and deletes a server
    void udcDeleteServer(UdcServer* server);

    // Try to connect to a client from a server
    // timeout is time in milliseconds
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

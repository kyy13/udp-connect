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

    // A unique identifier for a service
    struct UdcDeviceId
    {
        uint8_t bytes[20];
    };

    // Packet signature
    using UdcSignature = uint32_t;

//    enum UdcStatus : uint8_t
//    {
//        UDC_NOT_CONNECTED,
//        UDC_CONNECTING,
//        UDC_RECONNECTING,
//        UDC_CONNECTED,
//    };
//
//    enum UdcReliability : uint8_t
//    {
//        UDC_UNRELIABLE,
//        UDC_RECEIVED_AT_LEAST_ONCE,
//        UDC_RECEIVED_ONCE,
//        UDC_RECEIVED_IN_ORDER_ONCE,
//    };
//
//    enum UdcResult : uint32_t
//    {
//        UDC_SUCCESS,
//        UDC_BIND_SOCKET_ERROR,
//        UDC_ALREADY_CONNECTED,
//    };

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

    UdcEvent* udcProcessEvents(UdcServer* server);

    UdcEventType udcGetEventType(const UdcEvent* event);
}

#endif

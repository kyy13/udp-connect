// udp-connect
// Kyle J Burgess

#include "UdcSocketMux.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    UdcSignature sig = {{0x01, 0x02, 0x03, 0x04}};

    std::vector<uint8_t> message {'O', 'W', 'O'};
    std::vector<uint8_t> buffer(2048);

    // Create nodeA
    UdcServer* nodeA = udcCreateServer(sig, 1234, 2345, buffer.data(), buffer.size(), "test_unreliable_ipv6_logA.txt");

    if (nodeA == nullptr)
    {
        std::cout << "failed to create Node A\n";
        return -1;
    }

    // Create nodeB
    UdcServer* nodeB = udcCreateServer(sig, 1235, 2346, buffer.data(), buffer.size(), "test_unreliable_ipv6_logB.txt");

    if (nodeB == nullptr)
    {
        std::cout << "failed to create Node B\n";
        udcDeleteServer(nodeA);
        return -1;
    }

    // Connect nodeA to nodeB
    UdcEndPointId id;
    if (!udcTryConnect(nodeA, "::1", "1235", 1000, id))
    {
        std::cout << "failed to initiate connection from A to B\n";
        udcDeleteServer(nodeA);
        udcDeleteServer(nodeB);
        return -1;
    }

    // Receive until connected
    bool connected = false;
    const UdcEvent* event;

    auto t0 = std::chrono::system_clock::now();

    while (true)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            udcDeleteServer(nodeB);
            std::cout << "took too long.\n";
            return -1;
        }

        // Send from A to B once connected
        if (connected)
        {
            udcSendMessage(nodeA, id, message.data(), message.size(), UDC_UNRELIABLE_MESSAGE);
        }

        // Receive from nodeA
        while ((event = udcProcessEvents(nodeA)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
                case UDC_EVENT_CONNECTION_SUCCESS:
                    connected = true;
                    break;
                case UDC_EVENT_CONNECTION_TIMEOUT:
                    std::cout << "connection timed out\n";
                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    return -1;
                default:
                    break;
            }
        }

        // Receive from nodeB
        while ((event = udcProcessEvents(nodeB)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
                case UDC_EVENT_RECEIVE_MESSAGE_IPV6:
                {
                    UdcAddressIPv6 ip;
                    uint16_t port;
                    uint32_t index;
                    uint32_t size;

                    if (!udcGetResultMessageIPv6Event(event, ip, port, index, size))
                    {
                        std::cout << "couldn't read external ipv6 event\n";
                        udcDeleteServer(nodeA);
                        udcDeleteServer(nodeB);
                        return -1;
                    }

                    if (memcmp(message.data(), buffer.data() + index, size) != 0)
                    {
                        std::cout << "message wasn't the same\n";
                        udcDeleteServer(nodeA);
                        udcDeleteServer(nodeB);
                        return -1;
                    }

                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    return 0;
                }
                default:
                    break;
            }
        }
    }
}

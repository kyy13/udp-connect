// udp-connect
// Kyle J Burgess

#include "UdcSocketMux.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    constexpr uint32_t totalMessages = 10000;

    UdcSignature sig = {{0x01, 0x02, 0x03, 0x04}};

    uint32_t sentMessage = 0;
    uint32_t expectedMessage = 0;
    std::vector<uint8_t> buffer(2048);

    // Create nodeA
    UdcServer* nodeA = udcCreateServer(sig, buffer.data(), buffer.size(), "test_reliable_ipv6_logA.txt");

    if (nodeA == nullptr)
    {
        std::cout << "failed to create Node A\n";
        return -1;
    }

    if (!udcTryBindIPv6(nodeA, 1234))
    {
        std::cout << "failed to bind Node A\n";
        return -1;
    }

    // Create nodeB
    UdcServer* nodeB = udcCreateServer(sig, buffer.data(), buffer.size(), "test_reliable_ipv6_logB.txt");

    if (nodeB == nullptr)
    {
        std::cout << "failed to create Node B\n";
        udcDeleteServer(nodeA);
        return -1;
    }

    if (!udcTryBindIPv6(nodeB, 1235))
    {
        std::cout << "failed to bind Node B\n";
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
        if (connected && sentMessage < totalMessages)
        {
            udcSendMessage(nodeA, id, reinterpret_cast<uint8_t*>(&sentMessage), sizeof(sentMessage), UDC_RELIABLE_MESSAGE);
            ++sentMessage;
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
                        std::cout << "couldn't read external ipv4 event\n";
                        udcDeleteServer(nodeA);
                        udcDeleteServer(nodeB);
                        return -1;
                    }

                    if (memcmp(&expectedMessage, buffer.data() + index, size) != 0)
                    {
                        std::cout << "message wasn't the same\n";
                        udcDeleteServer(nodeA);
                        udcDeleteServer(nodeB);
                        return -1;
                    }
                    ++expectedMessage;

                    if (expectedMessage >= totalMessages)
                    {
                        udcDeleteServer(nodeA);
                        udcDeleteServer(nodeB);
                        return 0;
                    }
                }
                default:
                    break;
            }
        }
    }
}

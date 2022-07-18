// udp-connect
// Kyle J Burgess

#include "UdcSocketMux.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    UdcSignature sig = {{0x01, 0x02, 0x03, 0x04}};

    std::vector<uint8_t> buffer(2048);

    // Create nodeA
    UdcServer* nodeA = udcCreateServer(sig, buffer.data(), buffer.size(), "test_reconnect_ipv4_logA.txt");

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
    UdcServer* nodeB = udcCreateServer(sig, buffer.data(), buffer.size(), "test_reconnect_ipv4_logB.txt");

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
    const UdcEvent* event;

    auto t0 = std::chrono::system_clock::now();
    bool connected = false;

    while (!connected)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            udcDeleteServer(nodeB);
            std::cout << "failed to fully connect\n";
            return -1;
        }

        // Receive from nodeA
        while ((event = udcProcessEvents(nodeA)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
                case UDC_EVENT_CONNECTION_SUCCESS:
                    connected = true;
                    break;
                default:
                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    std::cout << "received event other than connection success\n";
                    return -1;
            }
        }

        // Receive from nodeB
        while (udcProcessEvents(nodeB) != nullptr);
    }

    // Stop processing nodeB
    // and allow connection to be lost
    t0 = std::chrono::system_clock::now();

    while (connected)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            udcDeleteServer(nodeB);
            std::cout << "failed to lose connection\n";
            return -1;
        }

        // Receive from nodeA
        while ((event = udcProcessEvents(nodeA)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
                case UDC_EVENT_CONNECTION_LOST:
                    connected = false;
                    break;
                default:
                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    std::cout << "received event other than connection lost\n";
                    return -1;
            }
        }

        // ! Don't process nodeB
    }

    // Start processing nodeB again until reconnection

    t0 = std::chrono::system_clock::now();

    while (true)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            udcDeleteServer(nodeB);
            std::cout << "failed to reconnect\n";
            return -1;
        }

        // Receive from nodeA
        while ((event = udcProcessEvents(nodeA)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
                case UDC_EVENT_CONNECTION_REGAINED:
                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    return 0;
                default:
                    udcDeleteServer(nodeA);
                    udcDeleteServer(nodeB);
                    std::cout << "received event other than connection regained\n";
                    return -1;
            }
        }

        // Receive from nodeB
        while (udcProcessEvents(nodeB) != nullptr);
    }
}

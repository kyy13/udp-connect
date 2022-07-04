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
    UdcServer* nodeA = udcCreateServer(sig, 1234, 2345, buffer.data(), buffer.size(), "test_timeout_ipv4_logA.txt");

    if (nodeA == nullptr)
    {
        std::cout << "failed to create Node A\n";
        return -1;
    }

    // Connect nodeA to another port that's not running a server, so it will time out
    UdcEndPointId id;
    if (!udcTryConnect(nodeA, "127.0.0.1", "2346", 1000, id))
    {
        std::cout << "failed to initiate connection from A to B\n";
        udcDeleteServer(nodeA);
        return -1;
    }

    // Receive until connected
    const UdcEvent* event;

    auto t0 = std::chrono::system_clock::now();

    while (true)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            std::cout << "failed to call timeout\n";
            return -1;
        }

        // Receive from nodeA
        while ((event = udcProcessEvents(nodeA)) != nullptr)
        {
            switch(udcGetEventType(event))
            {
            case UDC_EVENT_CONNECTION_SUCCESS:
                udcDeleteServer(nodeA);
                std::cout << "somehow connected?\n";
                return -1;
            case UDC_EVENT_CONNECTION_TIMEOUT:
                udcDeleteServer(nodeA);
                return 0;
            default:
                break;
            }
        }
    }
}

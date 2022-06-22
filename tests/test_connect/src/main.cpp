// udp-connect
// Kyle J Burgess

#include "UdcDualSocket.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    // Create nodeA
    UdcServer* nodeA = udcCreateServer(0x01020304, 1234, 2345);

    if (nodeA == nullptr)
    {
        std::cout << "failed to create Node A\n";
        return -1;
    }

    // Create nodeB
    UdcServer* nodeB = udcCreateServer(0x01020304, 1235, 2346);

    if (nodeB == nullptr)
    {
        std::cout << "failed to create Node B\n";
        return -1;
    }

    // Connect nodeA to nodeB
    if (!udcTryConnect(nodeA, "127.0.0.1", "1235", 1000))
    {
        std::cout << "failed to initiate connection from A to B\n";
        return -1;
    }

    // Receive until connected
    UdcDeviceId clientId;
    uint32_t size;

    auto t0 = std::chrono::system_clock::now();

    while (true)
    {
        auto t1 = std::chrono::system_clock::now();
        auto dt = t1 - t0;

        if (dt >= std::chrono::seconds(5))
        {
            udcDeleteServer(nodeA);
            udcDeleteServer(nodeB);
            return -1;
        }

        // Receive from nodeA
        uint8_t* result = udcReceive(nodeA, clientId, size);

        // Receive from nodeB
        result = udcReceive(nodeB, clientId, size);

        // Check if A is connected to B
        if (udcGetConnectionCount(nodeA) == 1)
        {
            break;
        }
    }

    udcDeleteServer(nodeA);
    udcDeleteServer(nodeB);

    return 0;
}

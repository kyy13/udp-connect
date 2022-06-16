// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcSocketReceiver.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    const uint8_t udpMessage[] = {'H', 'E', 'L', 'L', 'O', '!'};

    UdcAddressIPv6 loopBack {};
    if (!udcStringToIPv6("0:0:0:0:0:0:0:1", &loopBack))
    {
        std::cout << "failed to convert ip string\n";
        return -1;
    }

    UdcSocketReceiver usr(100);
    UdcSocketSender uss;

    // Connect sender

    if (!uss.connect(loopBack, 1235))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage, sizeof(udpMessage)))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (without calling disconnect)

    if (!uss.connect(loopBack, 4322))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage, sizeof(udpMessage)))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (with calling disconnect)

    uss.disconnect();

    if (!uss.connect(loopBack, 7778))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage, sizeof(udpMessage)))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect receiver

    if (!usr.connect(7778))
    {
        std::cout << "failed to connect socket receiver\n";
        return -1;
    }

    // Test sending and receiving many

    UdcAddressFamily family;
    UdcAddressIPv4 ipv4;
    UdcAddressIPv6 ipv6;
    UdcSocketReceiver::Buffer message;

    size_t messageSize;

    bool received = false;

    for (size_t i = 0; i != 100; ++i)
    {
        if (!uss.send(udpMessage, sizeof(udpMessage)))
        {
            std::cout << "failed to send.\n";
            return -1;
        }

        if (usr.receive(family, ipv4, ipv6, message, messageSize))
        {
            if (std::memcmp(message, udpMessage, sizeof(udpMessage)) != 0)
            {
                std::cout << "packet received incorrectly.\n";
                return -1;
            }

            received = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    if (!received)
    {
        std::cout << "packet not received.\n";
        return -1;
    }

    return 0;
}

// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcSocketReceiver.h"

#include <cstring>
#include <iostream>

int main()
{
    const uint8_t udpMessage[] = {'H', 'E', 'L', 'L', 'O', '!'};

    UdcAddressIPv4 loopBack {};
    if (!udcStringToIPv4("127.0.0.1", &loopBack))
    {
        std::cout << "failed to convert ip string\n";
        return -1;
    }

    UdcSocketReceiver usr(100);
    UdcSocketSender uss;

    // Connect sender

    if (!uss.connect(7777, loopBack, 1234))
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

    if (!uss.connect(7777, loopBack, 4321))
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

    if (!uss.connect(7777, loopBack, 7777))
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

    if (!usr.connect(7777))
    {
        std::cout << "failed to connect socket receiver\n";
        return -1;
    }

    // Test sending and receiving many

    UdcAddressFamily family;
    UdcAddressIPv4 ipv4;
    UdcAddressIPv6 ipv6;
    UdcSocketReceiver::Buffer message;

    uint16_t port;
    size_t messageSize;

    bool received = false;

    for (size_t i = 0; i != 1000; ++i)
    {
        if (!uss.send(udpMessage, sizeof(udpMessage)))
        {
            std::cout << "failed to send.\n";
            return -1;
        }

        if (usr.receive(family, ipv4, ipv6, port, message, messageSize))
        {
            if (family != UDC_IPV4)
            {
                std::cout << "ip family received incorrectly.\n";
                return -1;
            }

            if (std::memcmp(loopBack.octets, ipv4.octets, sizeof(ipv4.octets)) != 0)
            {
                std::cout << "packet ip received incorrectly.\n";
                return -1;
            }

            if (port != 7777)
            {
                std::cout << "port received is incorrect: " << port << '\n';
                return -1;
            }

            if (std::memcmp(message, udpMessage, sizeof(udpMessage)) != 0)
            {
                std::cout << "packet received incorrectly.\n";
                return -1;
            }

            received = true;
        }
    }

    if (!received)
    {
        std::cout << "packet not received.\n";
        return -1;
    }

    return 0;
}

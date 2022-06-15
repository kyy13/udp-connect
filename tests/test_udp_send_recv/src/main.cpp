// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcSocketReceiver.h"

#include <cstring>
#include <iostream>

int main()
{
    const char UDP_MESSAGE[] = "Hello Socket!";

    UdcSocketReceiver usr(100);
    UdcSocketSender uss;

    // Connect sender

    if (!uss.connect("127.0.0.1", 7777, 1234))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(UDP_MESSAGE))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (without calling disconnect)

    if (!uss.connect("127.0.0.1", 7777, 4321))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(UDP_MESSAGE))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (with calling disconnect)

    uss.disconnect();

    if (!uss.connect("127.0.0.1", 7777, 7777))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(UDP_MESSAGE))
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

    std::string ip;
    u_short port;
    std::vector<uint8_t> msg;
    bool received = false;

    for (size_t i = 0; i != 1000; ++i)
    {
        if (!uss.send(UDP_MESSAGE))
        {
            std::cout << "failed to send.\n";
            return -1;
        }

        if (usr.receive(ip, port, msg))
        {
            if (ip != "127.0.0.1")
            {
                std::cout << "packet ip received incorrectly.\n";
                return -1;
            }

            if (port != 7777)
            {
                std::cout << "port received is incorrect: " << port << '\n';
                return -1;
            }

            if (std::memcmp(msg.data(), UDP_MESSAGE, strlen(UDP_MESSAGE)) != 0)
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

// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"
#include "UdcSocketReceiver.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    std::vector<uint8_t> udpMessage = {'H', 'E', 'L', 'L', 'O', '!'};

    std::string loopBackIp = "0:0:0:0:0:0:0:1";

    UdcSocketReceiver usr;
    UdcSocketSender uss;

    // Connect sender

    if (!uss.connect(loopBackIp, "1234"))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (without calling disconnect)

    if (!uss.connect(loopBackIp, "4321"))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (with calling disconnect)

    uss.disconnect();

    if (!uss.connect(loopBackIp, "7777"))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!uss.send(udpMessage))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect receiver

    if (!usr.bind(7777, 7778))
    {
        std::cout << "failed to connect socket receiver\n";
        return -1;
    }

    // Test sending and receiving many

    UdcAddressFamily addressFamily;
    UdcSocketReceiver::UdcAddress address;
    std::vector<uint8_t> message;

    bool received = false;

    for (size_t i = 0; i != 100; ++i)
    {
        if (!uss.send(udpMessage))
        {
            std::cout << "failed to send.\n";
            return -1;
        }

        if (usr.receive(addressFamily, address, message))
        {
            if (!std::equal(udpMessage.begin(), udpMessage.end(), message.begin()))
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

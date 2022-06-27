// udp-connect
// Kyle J Burgess

#include "UdcSocketMux.h"

#include <cstring>
#include <iostream>
#include <thread>

int main()
{
    std::vector<uint8_t> udpMessage = {'H', 'E', 'L', 'L', 'O', '!'};

    UdcAddressIPv4 addressIPv4;
    uint16_t portIPv4;

    if (!UdcSocket::stringToIPv4("127.0.0.1", "1234", addressIPv4, portIPv4))
    {
        std::cout << "failed convert ip string to ip\n";
        return -1;
    }

    UdcSocketMux socket;

    // Connect sender

    if (!socket.bind(portIPv4 + 1, portIPv4))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!socket.send(addressIPv4, 7777, udpMessage.data(), udpMessage.size()))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (without calling disconnect)
    if (!socket.bind(portIPv4 + 1, portIPv4 + 2))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!socket.send(addressIPv4, 7777, udpMessage.data(), udpMessage.size()))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect to a different port (with calling disconnect)

    socket.disconnect();

    if (!socket.bind(portIPv4 + 1, portIPv4 + 3))
    {
        std::cout << "failed to connect socket sender\n";
        return -1;
    }

    if (!socket.send(addressIPv4, 7777, udpMessage.data(), udpMessage.size()))
    {
        std::cout << "failed to send\n";
        return -1;
    }

    // Connect receiver

    UdcSocketMux receiver;

    if (!receiver.bind(6666, 7777))
    {
        std::cout << "failed to connect socket receiver\n";
        return -1;
    }

    // Test sending and receiving many

    UdcAddressIPv4 rAddress;
    uint16_t rPort;
    std::vector<uint8_t> message(2048);

    for (size_t i = 0; i != 100; ++i)
    {
        if (!socket.send(addressIPv4, 7777, udpMessage.data(), udpMessage.size()))
        {
            std::cout << "failed to send.\n";
            return -1;
        }

        uint32_t msgSize = message.size();
        if (receiver.receive(rAddress, rPort, message.data(), msgSize))
        {
            if (!std::equal(udpMessage.begin(), udpMessage.end(), message.begin()))
            {
                std::cout << "packet received incorrectly.\n";
                return -1;
            }
            else
            {
                return 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    std::cout << "packet not received\n";
    return -1;
}

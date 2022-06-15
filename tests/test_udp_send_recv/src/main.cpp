// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"

int main()
{
    UdcSocketSender uss;

    // Connect
    if (!uss.connect("127.0.0.1", 1234))
    {
        return -1;
    }

    if (!uss.send("Hello Socket!"))
    {
        return -1;
    }

    // Connect to a different port (without calling disconnect)
    if (!uss.connect("127.0.0.1", 4321))
    {
        return -1;
    }

    if (!uss.send("Hello Socket!"))
    {
        return -1;
    }

    // Connect to a different port (with calling disconnect)

    uss.disconnect();

    if (!uss.connect("127.0.0.1", 5432))
    {
        return -1;
    }

    if (!uss.send("Hello Socket!"))
    {
        return -1;
    }

    return 0;
}

// udp-connect
// Kyle J Burgess

#include "UdcSocketSender.h"

int main()
{
    UdcSocketSender uss;

    if (!uss.connect("127.0.0.1", 1234))
    {
        return -1;
    }

    if (!uss.send("Hello Socket!"))
    {
        return -1;
    }

    return 0;
}

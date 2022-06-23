// udp-connect
// Kyle J Burgess

#include "UdcServer.h"

UdcServer::UdcServer()
    : id({})
{}

UdcServer::UdcServer(const std::string& logFileName)
    : id({})
    , socket(logFileName)
{}

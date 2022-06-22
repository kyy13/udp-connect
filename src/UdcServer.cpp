// udp-connect
// Kyle J Burgess

#include "UdcServer.h"

UdcClientInfo::UdcClientInfo()
    : id({})
{}

UdcServer::UdcServer()
    : id({})
{}

UdcServer::UdcServer(const std::string& logFileName)
    : id({})
    , socket(logFileName)
{}

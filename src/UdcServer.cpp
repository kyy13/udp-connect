// udp-connect
// Kyle J Burgess

#include "UdcServer.h"

UdcServer::UdcServer() = default;

UdcServer::UdcServer(const std::string& logFileName)
    : socket(logFileName)
{}

// udp-connect
// Kyle J Burgess

#ifndef UDC_COMMON_H
#define UDC_COMMON_H

#include <string>
#include <winsock2.h>

bool setSocketNonBlocking(SOCKET socket);

void convertToIp(const sockaddr_in* sa, std::string& ip);

#endif

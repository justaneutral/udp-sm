#ifndef __XXXX_NETWORK_INTERFACE__
#define __XXXX_NETWORK_INTERFACE__

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include "Logger.h"

class NetInterface {
public:
  NetInterface();
  ~NetInterface();
  //prototypes
  int SocketGetIPv4Address(const char * dev, char * ipv4);
  int SocketInitializeIPv4Receiver(char *m_addr_multicast,char *SocketIPv4Address,int SocketIPv4Port);
  int SocketUninitialize(void);
  Logger *LoggerPtr;
private:
  socklen_t SocketIPv4AddressLength;
  struct sockaddr_in GroupSocketInstance;
  int SocketIPv4SocketDatagram;
  struct ip_mreq xxx_group;
};

#endif //__XXXX_NETWORK_INTERFACE__

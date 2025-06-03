
#include "NetInterface.h"

NetInterface::NetInterface() {
  LoggerPtr = (Logger *)NULL;
}


NetInterface::~NetInterface() {}


int NetInterface::SocketGetIPv4Address(const char * dev, char * ipv4) {
  struct ifreq ifc;
  int res;
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd < 0) {
    return -1;
  }
  strcpy(ifc.ifr_name, dev);
  res = ioctl(sockfd, SIOCGIFADDR, &ifc);
  close(sockfd);
  if (res < 0) {
    return -1;
  }     
  strcpy(ipv4, inet_ntoa(((struct sockaddr_in*)&ifc.ifr_addr)->sin_addr));
  return 0;
}



int NetInterface::SocketInitializeIPv4Receiver(char *m_addr_multicast,char *SocketIPv4Address,int SocketIPv4Port) {
  int retcode = 0;
  SocketIPv4AddressLength = sizeof(GroupSocketInstance);
  SocketIPv4SocketDatagram = socket(AF_INET,SOCK_DGRAM,0);
  if (SocketIPv4SocketDatagram < 0) {
    LogWarn("Opening datagram socket error");
    retcode = -2;
  } else {
    LogInfo("Opening datagram socket....OK.\n");
    int flags = fcntl(SocketIPv4SocketDatagram,F_GETFL,0);
    fcntl(SocketIPv4SocketDatagram,F_SETFL,flags|O_NONBLOCK);
  }

  /* Enable SO_REUSEADDR to allow multiple instances of this */
  /* application to receive copies of the multicast datagrams. */
  if (retcode == 0) {
    const int reuse = 1;
    if (setsockopt(SocketIPv4SocketDatagram, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
      LogWarn("Setting SO_REUSEADDR error");
      close(SocketIPv4SocketDatagram);
      retcode = -3;
    } else {
      LogInfo("Setting SO_REUSEADDR...OK.\n");
    }
  }

  /* Bind to the proper port number with the IP address */
  /* specified as INADDR_ANY. */
  if(retcode == 0) {
    memset((char *) &GroupSocketInstance, 0, sizeof(GroupSocketInstance));
    GroupSocketInstance.sin_family = AF_INET;
    GroupSocketInstance.sin_port = htons(SocketIPv4Port);
    GroupSocketInstance.sin_addr.s_addr = INADDR_ANY;
    if(bind(SocketIPv4SocketDatagram, (struct sockaddr*)&GroupSocketInstance, sizeof(GroupSocketInstance))) {
      LogWarn("Binding datagram socket error");
      retcode = -4;
      close(SocketIPv4SocketDatagram);
    } else {
      LogInfo("Binding datagram socket...OK.\n");
    }
  }

  /* Join the multicast group 239.2.2.6 on the local 10.2.16.44 */
  /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
  /* called for each local interface over which the multicast */
  /* datagrams are to be received. */
  if(retcode == 0) {
    xxx_group.imr_multiaddr.s_addr = inet_addr(m_addr_multicast);
    xxx_group.imr_interface.s_addr = inet_addr(SocketIPv4Address);
    if(setsockopt(SocketIPv4SocketDatagram, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&xxx_group, sizeof(xxx_group)) < 0) {
      LogWarn("Adding multicast group error");
      close(SocketIPv4SocketDatagram);
      retcode = -5;
    } else {
      LogInfo("Adding multicast group...OK.\n");
      SocketIPv4AddressLength = sizeof(GroupSocketInstance);
    }
  }
  return retcode;
}


int NetInterface::SocketUninitialize(void) {
  close(SocketIPv4SocketDatagram);
  return 0;
}

// _eof_


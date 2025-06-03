
#ifndef __STREAM_RECEIVER_INTERFACE__H__
#define __STREAM_RECEIVER_INTERFACE__H__

#include "Tsc.h"
#include "RingFIFOBuffer.h"
#include "CommonData.h"
#include "Logger.h"

/******************************************************************************/

class NetworkReceiverInterface {
public:
  virtual ~NetworkReceiverInterface() {};
  virtual int Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr, char *NetworkInterfaceName, int port, char *IPv4MulticastAddress, int ConfigDumpReceivedPacketsInHexadecimalFormat, int ConfigDumpReceivedPacketsInAsciiFormat) = 0;
  virtual void Iterate(void) = 0;
  virtual int Uninitialize(void) = 0;
};

#endif // __STREAM_RECEIVER_INTERFACE__H__
// _eof_

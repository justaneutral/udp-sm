#ifndef __PUBLISHER_DATA_INTERFACE__
#define __PUBLISHER_DATA_INTERFACE__

#include "CommonData.h"
#include "RingFIFOBuffer.h"
#include "Logger.h"
#include "Percentile.h"
#include "Tsc.h"

class  InserterInterface {
public:
  virtual ~InserterInterface() {}
  virtual int Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Percentile *PercentilePtr, Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr) = 0;
  virtual int Uninitialize(void) = 0;
  virtual void SetWriterFlags(uint32_t WriterFlagsVal) = 0;
  virtual void PrintSessionIdFromSharedMemory(void) = 0;
  virtual void WriteTimestampToSharedMemory(void) = 0;
  virtual void PrintSharedMemoryBlockPositions(void) = 0;
  virtual void Iterate(void) = 0;
};

#endif // __PUBLISHER_DATA_INTERFACE__


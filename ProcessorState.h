
#ifndef __PROCESSOR_STATE_H__
#define __PROCESSOR_STATE_H__

#include "utils.h"
#include <sched.h>
#include "Logger.h"

class ProcessorState {
public:
  ProcessorState(Logger *LoggerPtr);
  ~ProcessorState();
  void PrintProcessorCoreAffinity(void);
  int StickThisThreadToProcessorCore(int CpuCoreNumber);
  Logger *LoggerPtr;
};

#endif // __PROCESSOR_STATE_H__

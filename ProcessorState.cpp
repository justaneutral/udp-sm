
//#define _GNU_SOURCE

#include "ProcessorState.h"


ProcessorState::ProcessorState(Logger *LoggerPtr) {
  this->LoggerPtr = LoggerPtr;
}

ProcessorState::~ProcessorState() {
  munlockall();
}


void ProcessorState::PrintProcessorCoreAffinity(void)
{
  cpu_set_t mask;
  long nproc, i;

  if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
    LogError("ERROR in sched_getaffinity\n");
  } else {
    nproc = sysconf (_SC_NPROCESSORS_ONLN);
    LogInfo("sched_getaffinity = ");
    for (i = 0; i < nproc; i++) {
      LogInfo("%d ", CPU_ISSET(i, &mask));
    }
    LogInfo("\n");
  }
}


int ProcessorState::StickThisThreadToProcessorCore(int CpuCoreNumber) {
  int ReturnValue = 0;
  //we can set one or more bits here, each one representing a single CPU
  cpu_set_t cpuset; 
  CPU_ZERO(&cpuset);       //clears the cpuset
  CPU_SET(CpuCoreNumber , &cpuset); //set CPU 2 on cpuset
  /*
  * cpu affinity for the calling thread 
  * first parameter is the pid, 0 = calling thread
  * second parameter is the size of your cpuset
  * third param is the cpuset in which your thread will be
  * placed. Each bit represents a CPU
  */
  ReturnValue = sched_setaffinity(0, sizeof(cpuset), &cpuset);
  if (-1 == ReturnValue) {
    LogWarn("WARNING: sched_setaffinity returns %d\n", ReturnValue);
  } else {
    PrintProcessorCoreAffinity();
    // Pin all pages mapped to the process in memory
    LogDebug("Pin all the pages in memory.\n");
    if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
      LogError("mlockall error");
      ReturnValue = -1;
    }
  }
  return ReturnValue;
}

// _eof_


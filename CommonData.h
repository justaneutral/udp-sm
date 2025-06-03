#ifndef __COMMON_DATA_H__
#define __COMMON_DATA_H__

#include "defines.h"

typedef struct SessionIdStruct_t {
  uint8_t  prefix[4];
  uint32_t seconds;
} __attribute__((packed)) SessionIdS_t;

typedef union SessionIdUnion_t {
  SessionIdS_t  Struct;
  uint64_t      Binary;
} __attribute__((packed)) SessionIdU_t;

class CommonData {
public:
  CommonData(int Verbose, char *SharedMemoryName, char *LogFileName, int LoggingLevel) {
    ConfigurationFlags = 0;
    ApplicationRunFlag = 0;
    ConfigSleep = 0;
    ConfigThreshold = 0;
    CommonFlagConnected = 0;
    CommonFlagShutdownReceived = 0;
    CommonFlagSessionIdReceived = 0;
    CommonValueSessionId = 0;
    CommonValueSequenceNumber = 0;
    CommonValueSequenceNumber = 0;
    TotalNumberOfParsedFrames = 0;
    TotalNumberOfParsedMessages = 0;
    TotalNumberOfReceivedMessages = 0;
    TotalNumberOfRejectedMessages = 0;
    ProcessorCoreId = 0;
    this->Verbose = Verbose;
    if (SharedMemoryName) strncpy(this->SharedMemoryName, SharedMemoryName, __SHM_NAME_LENGTH__);
    if (LogFileName) strncpy(this->LoggingFileName, LogFileName, LOGNAMSIZ);
    this->LoggingLevel = LoggingLevel;
  }

  ~CommonData() {}

  int        ApplicationRunFlag;
  int        ConfigurationFlags;
  int        Verbose;
  int        ConfigSleep;
  int        ConfigThreshold;
  int        CommonFlagConnected;
  int        CommonFlagShutdownReceived;
  int        CommonFlagSessionIdReceived;
  uint64_t   CommonValueSessionId;
  uint64_t   CommonValueSequenceNumber;
  uint64_t   TotalNumberOfParsedFrames;
  uint64_t   TotalNumberOfParsedMessages;
  uint64_t   TotalNumberOfReceivedMessages;
  uint64_t   TotalNumberOfRejectedMessages;
  int        ProcessorCoreId;
  char       SharedMemoryName[__SHM_NAME_LENGTH__];
  char       LoggingFileName[LOGNAMSIZ];
  int        LoggingLevel;
};

#endif // __COMMON_DATA_H__


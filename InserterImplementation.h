#ifndef __PUBLISHER_DATA__
#define __PUBLISHER_DATA__

#include <iostream>
#include <atomic>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "utils.h"
#include "defines.h"
#include "flags.h"
#include "CommonData.h"
#include "Logger.h"
#include "Tsc.h"
#ifdef __CONTROLABLE__
#include "Percentile.h"
#endif // __CONTROLABLE__

#include "InserterInterface.h"
#include "InserterSharedMemoryFormatDefinitions.h"

class  InserterImplementation : public InserterInterface {
public:
  InserterImplementation();
  ~InserterImplementation();
  int Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Percentile *PercentilePtr, Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr);
  int Uninitialize(void);
  void SetWriterFlags(uint32_t WriterFlagsVal);
private:
  void UpdateWriterFlags(void);
  void WriteSessionIdToSharedMemory(void);
public:
  void PrintSessionIdFromSharedMemory(void);
private:
#if(0)
  void ShmInitWaitForSession(void);
#endif
  void ShowInserterPointers(void);
  void ShowInserterData(void);
  void TimingStatisticsCollection(void);
public:
  void WriteTimestampToSharedMemory(void);
private:
  void WriteFileMarkerToSharedMemory(void);
  char *ReadFileMarkerFromSharedMemory(void);
public:
  void HandleSharedMemoryFrames(void);
private:
  char *ParseWriterFlags(uint32_t v);
public:
  void PrintSharedMemoryBlockPositions(void);
private:
  void PrintFileControlBlockPositions(void);
  void PrintFileConfigurationInformationPositions(void);
  void PrintWriterStateBlockPositions(void);
  void PrintDataSectionPositions(void);
  void PrintCurrentMessagePositions(void);
public:
  //void ReadKeyboardInput(void);
private:
  int ReadParameters(int argc, char *argv[]);
  int OpenSharedMemoryFile(int *SharedMemoryFileId, void **SharedMemoryBufferPtr, size_t SharedMemoryAllocationSize, char *SharedMemoryName);
  void CloseSharedMemoryFile(void);
  int DoChangeFileOwner(const char *FilePathName, const char *UserName, const char *GroupName);
  template <typename T> void WriteMemory(size_t NumberOfOffsetBytes, T Value);
  template <typename T> T ReadMemory(size_t NumberOfOffsetBytes);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __USE_DEFINES_FOR_SHARED_MEMORY_ACCESS__
  void WriteFileVersion(int32_t v) { WriteMemory<int32_t>((__FileVersionPosition__), (v));}
  uint32_t ReadFileVersion(void) { return ReadMemory<uint32_t>((__FileVersionPosition__));}
  void WriteLayoutFlags(uint32_t v) { WriteMemory<int32_t>((__LayoutFlagsPosition__), (v));}
  uint32_t ReadLayoutFlags(void) { return ReadMemory<uint32_t>((__LayoutFlagsPosition__));}
  void WritePageSize(uint32_t v) { WriteMemory<uint32_t>((__PageSizePosition__), (v));}
  uint32_t ReadPageSize(void) { return ReadMemory<uint32_t>((__PageSizePosition__));}
  void WriteLineSize(uint32_t v) { WriteMemory<uint32_t>((__LineSizePosition__), (v));}
  uint32_t ReadLineSize(void) { return ReadMemory<uint32_t>((__LineSizePosition__));}
  void WriteDataSize(uint32_t v) { WriteMemory<uint32_t>((__DataSizePosition__), (v));}
  uint32_t ReadDataSize(void) { return ReadMemory<uint32_t>((__DataSizePosition__));}
  void WriteDataOffset(uint64_t v) { WriteMemory<uint64_t>((__DataOffsetPosition__), (v));}
  uint64_t ReadDataOffset(void) { return ReadMemory<uint64_t>((__DataOffsetPosition__));}
  // writer_state_information - cache line aligned
  void WriteWriterHeartbeatTime(uint64_t v) { WriteMemory<uint64_t>((__WriterHeartbeatTimePosition__), (v));}
  uint64_t ReadWriterHeartbeatTime(void) { return ReadMemory<uint64_t>((__WriterHeartbeatTimePosition__));}
  void WriteWriterSessionId(uint64_t v) { WriteMemory<uint64_t>((__WriterSessionIdPosition__), ntoh64(v));}
  uint64_t ReadWriterSessionId(void) { return hton64(ReadMemory<uint64_t>((__WriterSessionIdPosition__)));}
  void WriteWriterMaxSequence(uint64_t v) { WriteMemory<uint64_t>((__WriterMaxSequencePosition__), (v));}
  uint64_t ReadWriterMaxSequence(void) { return ReadMemory<uint64_t>((__WriterMaxSequencePosition__));}
  void WriteWriterFlags(uint32_t v) { WriteMemory<uint32_t>((__WriterFlagsPosition__), (v));}
  uint32_t ReadWriterFlags(void) { return ReadMemory<uint32_t>((__WriterFlagsPosition__));}
  void WriteCommitSequence(uint64_t v) { WriteMemory<uint64_t>((__CommitSequencePosition__), (v));}
  uint64_t ReadCommitSequence(void) { return ReadMemory<uint64_t>((__CommitSequencePosition__));}
  void WriteReserveSequence(uint64_t v) { WriteMemory<uint64_t>((__ReserveSequencePosition__), (v));}
  uint64_t ReadReserveSequence(void) { return ReadMemory<uint64_t>((__ReserveSequencePosition__));}
#endif // __USE_DEFINES_FOR_SHARED_MEMORY_ACCESS__
  ///////////////////////////////////////////////////////////////////////////////////////////////////////
public:
  void Iterate(void);
  template <typename T> T ReverseBytes(T value);
  template <typename T> T HostToNetwork(T value);
  template <typename T> T NetworkToHost(T value);

private:
  const int32_t SharedMemoryFileVersion = 1;
  const int32_t SharedMemoryLayoutFlags = 0;  //reserved for future use, now always 0.
  uint64_t SharedMemoryFileDataOffset;
  size_t SharedMemoryAllocationSize;
  uint64_t            SecondsSinceProgramStart;
public:
  uint64_t            NumberOfInsertedFramesSinceProgramStart;
private:
  //unsigned long long  MillisecondsSinceEpoch;
  uint64_t            MillisecondsSinceEpoch;
public:
  uint64_t            CommitSequenceValue;
  uint64_t            ReserveSequenceValue;
private:
  uint64_t            MaxSequenceValue;
  int                 SharedMemoryFileId;
public:
  void                *SharedMemoryBufferPtr;
private:
  uint32_t            WriterFlagsPrevious;
public:
  uint32_t            WriterFlags;
  uint64_t            FramingMetaDataMessageSequenceNumber; //int64
  uint64_t            CurrentCacheLine;
  uint64_t            CacheLinesRemaining;
  uint16_t            FramingMetaDataTotalLines;
  int                 ProcessedFrameNumber;
  uint16_t            *FramingMetaDataTotalLinesPtr;
  uint16_t            FramingMetaDataTotalNumberOfMessageBytes;
  uint16_t            *FramingMetaDataTotalNumberOfMessageBytesPtr;
  uint64_t            *FramingMetaDataMessageSequenceNumberPtr;
  void                *FramingMetaDataMessagePayloadPtr;
  void                *MaxSequencePtr;
#ifdef __ATOMIC__
  void  *SharedMemoryWriterFlagsPtr;
  void  *SharedMemoryWriterSessionIdPtr;
  void  *SharedMemoryCommitSequencePtr;
  void  *SharedMemoryReserveSequencePtr;
  std::atomic<uint32_t>   *AtomicWriterFlagsPtr;
  std::atomic<uint64_t>   *AtomicWriterSessionIdPtr;
  std::atomic<uint64_t>   *AtomicCommitSequencePtr;
  std::atomic<uint64_t>   *AtomicReserveSequencePtr;
  std::atomic<uint64_t>   *AtomicMaxSequencePtr;
#endif // __ATOMIC__
#ifdef __CONTROLABLE__
  Percentile        *PercentilePtr;
  Tsc *TscPtr;
#endif // __CONTROLABLE__
  uint8_t *ReceivedDataPtr;
  uint8_t *ReceivedDataBuffer;
  Logger *LoggerPtr;
  CommonData *CommonDataPtr;
  RingFIFOBuffer *RingBufferPtr;
  int ReturnValue;
};

#endif // __PUBLISHER_DATA__


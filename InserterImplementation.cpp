
#include "InserterImplementation.h"

//#define __TSC_DONT_MEASURE_OVERHEAD__

////////////////

// -- 4 -- populate the Control Block with all of the relevant file layout information fields (marker, version, etc)
void InserterImplementation::WriteFileMarkerToSharedMemory(void) {
  memcpy((void *)((uint8_t*)SharedMemoryBufferPtr + __FileMarkerPosition__), (const void *)"_CB_", (size_t)4);
}

char *InserterImplementation::ReadFileMarkerFromSharedMemory(void) {
  static char s[5];
  memset((void *)s, (int)0, (size_t)5);
  memcpy((void *)s, (const void *)((uint8_t *)SharedMemoryBufferPtr + __FileMarkerPosition__), (size_t)4);
  return s;
}
// -- 5 -- populate the File Control Block with the current writer time, session ID of 0, flags of 0, max sequence of 0
// -- 6 -- initialize the Commit and Reserve Sequence fields to 0
// -- 7 -- begin a timer for updating the writer timestamp field in the File Control Block
void InserterImplementation::WriteTimestampToSharedMemory(void) {
  if (CommonDataPtr->ApplicationRunFlag) {
    struct timeval TimeValue;
    gettimeofday(&TimeValue, NULL);
    uint64_t MillisecondsSinceEpochCurrent = (uint64_t)(TimeValue.tv_sec) * 1000 + (uint64_t)(TimeValue.tv_usec) / 1000;

    if (MillisecondsSinceEpochCurrent - MillisecondsSinceEpoch >= 1000) {
      MillisecondsSinceEpoch = MillisecondsSinceEpochCurrent;
      WriteWriterHeartbeatTime(MillisecondsSinceEpochCurrent);
      //At the end of the current event loop tick, write the Max Sequence Number and the Commit Sequence into the Control Header with sequential consistency semantics
#ifdef __ATOMIC__
#ifdef __SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
      AtomicMaxSequencePtr->store(MaxSequenceValue,std::memory_order_seq_cst);
#else
      AtomicMaxSequencePtr->store(hton64(MaxSequenceValue),std::memory_order_seq_cst);
#endif
#else
#ifdef __WRITE_LITTLE_ENDIAN__
      AtomicMaxSequencePtr->store(MaxSequenceValue,std::memory_order_release);
#else
      AtomicMaxSequencePtr->store(hton64(MaxSequenceValue),std::memory_order_release);
#endif
#endif
#else
      WriteWriterMaxSequence((void *)SharedMemoryBufferPtr, MaxSequenceValue);
#endif
//#ifdef __CONTROLABLE__
#ifndef __TEST_PUBLISHER_ONLY__  
      if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_HEARTBEAT_)
#endif
      {
        LogInfo("\n\t::heartbeat_time(ms)\t=\t%ld\t@\t%llu\tMaxSequenceValue\t=\t%lu\t@\t%llu\n", ReadWriterHeartbeatTime(), __WriterHeartbeatTimePosition__, ReadWriterMaxSequence(), __WriterMaxSequencePosition__);
      }
      SecondsSinceProgramStart++;
//#endif //__CONTROLABLE__
    }
  }
  return; // NULL;
}


void InserterImplementation::TimingStatisticsCollection(void) {
#ifdef __CONTROLABLE__
  //tsc
  if (TscPtr && (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_)) {//1
    TscPtr->BenchEnd();
    uint64_t latency = TscPtr->GetLatency();
    if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_TIMING_) {//2
      LogFprintfInfo("Latency = %lu\n", latency);
    } else {//2
      PercentilePtr->AddNanosecondTimeStampMeasurement(latency);
    }//2
  }//1
#endif //__CONTROLABLE__
}


void InserterImplementation::ShowInserterData(void) {
//#ifdef __CONTROLABLE__
  if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_DATA_) {//1
    LogInfo("\rInserter received %d data bytes:", FramingMetaDataTotalNumberOfMessageBytes);
    for (int i = 0; i <  FramingMetaDataTotalNumberOfMessageBytes; i++) {//2
      LogInfo(" %02X", ReceivedDataBuffer[i]);
    }//2
    LogInfo("\n");
    //fflush(stdout);
  }//1
//#endif // __CONTROLABLE__
}


void InserterImplementation::ShowInserterPointers(void) {
//#ifdef __CONTROLABLE__
  if (CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PUBLISHER_) {//1
    LogInfo("\nseq\t=\t%ld\tbytes\t=\t%d\tlines\t=\t%d\n", FramingMetaDataMessageSequenceNumber, FramingMetaDataTotalNumberOfMessageBytes, FramingMetaDataTotalLines);
    LogInfo("CurrentCacheLine\t=\t%lu\tCacheLinesRemaining\t=\t%lu\tFramingMetaDataTotalNumberOfMessageBytes\t=\t%u\tFramingMetaDataTotalLines=\t%d\n", CurrentCacheLine, CacheLinesRemaining, FramingMetaDataTotalNumberOfMessageBytes, FramingMetaDataTotalLines);
    LogInfo("Locals:\tReserveSequenceValue\t=\t%ld\tCommitSequenceValue\t=\t%ld\tMaxSequenceValue\t=\t%lu\n", ReserveSequenceValue, CommitSequenceValue, MaxSequenceValue);
    LogInfo("In SHM:\tReserveSequenceValue\t=\t%ld\tCommitSequenceValue\t=\t%ld\tMaxSequenceValue\t=\t%lu\n", ReadReserveSequence(), ReadCommitSequence(), ReadWriterMaxSequence());
#ifdef __ATOMIC__
#ifdef __WRITE_LITTLE_ENDIAN__
    LogInfo("Atomic:\tReserveSequenceValue\t=\t%ld\tCommitSequenceValue\t=\t%ld\tMaxSequenceValue\t=\t%ld (little endian)\n", AtomicReserveSequencePtr->load(), AtomicCommitSequencePtr->load(std::memory_order_acquire), AtomicMaxSequencePtr->load());
#else
    LogInfo("Atomic:\tReserveSequenceValue\t=\t%ld\tCommitSequenceValue\t=\t%ld\tMaxSequenceValue\t=\t%ld (big endian)\n", ntoh64(AtomicReserveSequencePtr->load()), ntoh64(AtomicCommitSequencePtr->load(std::memory_order_acquire)), ntoh64(AtomicMaxSequencePtr->load()));
#endif
#endif
  }//1
//#endif //__CONTROLABLE__
}


void InserterImplementation::PrintSessionIdFromSharedMemory(void) {
  SessionIdU_t SessionId;
  //process changes in writer flags in established session
#ifdef __ATOMIC__
  SessionId.Binary = AtomicWriterSessionIdPtr->load(std::memory_order_acquire);
#else //__ATOMIC__
  SessionId.Binary = ReadWriterSessionId.Struct.ared_memory_buffer_ptr);
#endif // __ATOMIC__
#ifndef __WRITE_LITTLE_ENDIAN__
  SessionId.Struct.seconds = ntohl(SessionId.Struct.seconds);
#endif // __WRITE_LITTLE_ENDIAN__
  LogDebug("SessionId\t\t=\t%c%c%c%c %u\n", SessionId.Struct.prefix[0], SessionId.Struct.prefix[1], SessionId.Struct.prefix[2], SessionId.Struct.prefix[3], SessionId.Struct.seconds);
}


void InserterImplementation::WriteSessionIdToSharedMemory(void) {
  //process changes in writer flags in established session
#ifdef __ATOMIC__
  AtomicWriterSessionIdPtr->store(CommonDataPtr->CommonValueSessionId, std::memory_order_seq_cst);
#else //__ATOMIC__
  WriteWriterSessionId.Struct.ared_memory_buffer_ptr, CommonDataPtr->CommonValueSessionId);
#endif // __ATOMIC__
}



void InserterImplementation::UpdateWriterFlags(void) {
  //process changes in writer flags in established session
  WriterFlags = __WriterFlagsConnected__ | (CommonDataPtr->CommonFlagShutdownReceived ?  __WriterFlagsShutdown__ : __WriterFlagsInSession__);
  if (WriterFlagsPrevious != WriterFlags) {//0
    WriterFlagsPrevious = WriterFlags;
#ifdef __ATOMIC__
#ifdef __SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(WriterFlags, std::memory_order_seq_cst);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(AtomicWriterFlagsPtr->load(std::memory_order_acquire)), __WriterFlagsPosition__);
#else // __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(htonl(WriterFlags), std::memory_order_seq_cst);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ntohl(AtomicWriterFlagsPtr->load(std::memory_order_acquire))), __WriterFlagsPosition__);
#endif // __WRITE_LITTLE_ENDIAN__
#else //__SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(WriterFlags, std::memory_order_release);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(AtomicWriterFlagsPtr->load(std::memory_order_acquire)), __WriterFlagsPosition__);
#else //__WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(htonl(WriterFlags), std::memory_order_release);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ntohl(AtomicWriterFlagsPtr->load(std::memory_order_acquire))), __WriterFlagsPosition__);
#endif // //__WRITE_LITTLE_ENDIAN__
#endif //__SEQ_CST__
#else // __ATOMIC__
    WriteWriterFlags(SharedMemoryBufferPtr, WriterFlags);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ReadWriterFlags(SharedMemoryBufferPtr)), __WriterFlagsPosition__);
#endif // __ATOMIC__
    WriteSessionIdToSharedMemory();
    PrintSessionIdFromSharedMemory();
  }//0
}



////////////////////////////////////////////////////////////////////////////////////

void InserterImplementation::HandleSharedMemoryFrames(void) {
  //process messages of an established session
  if (FramingMetaDataTotalNumberOfMessageBytes > 0) {//0
//#ifdef __CONTROLABLE__
    NumberOfInsertedFramesSinceProgramStart++;
//#endif //__CONTROLABLE__
#ifdef __SEQUENCE_NUMBER_LOCAL__
    FramingMetaDataMessageSequenceNumber++;
#else
#ifdef __SEQUENCE_NUMBER_QUEUE_SEQUENCE__
    FramingMetaDataMessageSequenceNumber = ntohl(((StrippedMessageHeaderS_t *)ReceivedDataBuffer)->QueueSequence);
#else
    FramingMetaDataMessageSequenceNumber = CommonDataPtr->CommonValueSequenceNumber;
#endif
#endif
    //The mechanism to append a message to the data section is as follows:
    //The current cache line to write at is the value of “Commit Sequence” divided by the total number of cache lines in the data section.
    CurrentCacheLine = CommitSequenceValue % __TOTAL_NUMBER_OF_DATA_CACHE_LINES__;
    //The number of contiguous cache lines remaining is the total number of cache lines in the data section minus the current cache line.
    CacheLinesRemaining = __TOTAL_NUMBER_OF_DATA_CACHE_LINES__ - CurrentCacheLine;
    //Calculate the required number of cache lines to store the message and the framing metadata
    FramingMetaDataTotalLines = (FramingMetaDataTotalNumberOfMessageBytes + sizeof(FramingMetaDataTotalLines) + sizeof(FramingMetaDataTotalNumberOfMessageBytes) + sizeof(FramingMetaDataMessageSequenceNumber) + __CACHE_LINE_SIZE__ -1) / __CACHE_LINE_SIZE__;

    //If not enough contiguous lines remain to hold the message,
    //write a “skip” section into the final lines of the data section ring buffer as follows:
    // and then write the message to the beginning of the ring buffer as described in the above step:
    if (FramingMetaDataTotalLines > CacheLinesRemaining) {//1
     //add the remaining number of lines to the Reserve Sequence, and write the field with at least store-release semantics.
      ReserveSequenceValue += CacheLinesRemaining;
#ifdef __ATOMIC__
#ifdef __WRITE_LITTLE_ENDIAN__
      AtomicReserveSequencePtr->store(ReserveSequenceValue, std::memory_order_release);
#else
      AtomicReserveSequencePtr->store(hton64(ReserveSequenceValue), std::memory_order_release);
#endif
#else
       WriteReserveSequence((void *)SharedMemoryBufferPtr, ReserveSequenceValue);
#endif
      //write a meta data block to the current cache line specifying the remaining cache lines as Total Lines Used, zero as the Total Bytes Used, and Zero as the Sequence Number (this indicates to the consumer this is a “skip” and not a valid message, as all valid messages have a nonzero sequence number).
          ///pddbg///memcpy((void *)((uint8_t *)SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (CurrentCacheLine * __CACHE_LINE_SIZE__)), (void *)&CacheLinesRemaining, sizeof(FramingMetaDataTotalLines));
      *(uint16_t *)((uint8_t *)SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (CurrentCacheLine * __CACHE_LINE_SIZE__)) = htons(CacheLinesRemaining);
      memset((void *)((uint8_t *)SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (CurrentCacheLine * __CACHE_LINE_SIZE__) + sizeof(FramingMetaDataTotalLines)), 0, sizeof(FramingMetaDataTotalNumberOfMessageBytes) + sizeof(FramingMetaDataMessageSequenceNumber));
      //add the number of lines skipped to the Commit Sequence. (This is also not committed at this time but instead at flush time at the end of the current event loop tick.
      CommitSequenceValue += CacheLinesRemaining;
#ifdef __ATOMIC__
#ifdef __SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
      AtomicCommitSequencePtr->store(CommitSequenceValue, std::memory_order_seq_cst);
#else // __WRITE_LITTLE_ENDIAN__
      AtomicCommitSequencePtr->store(hton64(CommitSequenceValue), std::memory_order_seq_cst);
#endif // __WRITE_LITTLE_ENDIAN__
#else //__SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
      AtomicCommitSequencePtr->store(CommitSequenceValue, std::memory_order_release);
#else // __WRITE_LITTLE_ENDIAN__
      AtomicCommitSequencePtr->store(hton64(CommitSequenceValue), std::memory_order_release);
#endif // __WRITE_LITTLE_ENDIAN__
#endif // __SEQ_CST__
#else // __ATOMIC__
      WriteCommitSequence((void *)SharedMemoryBufferPtr, CommitSequenceValue);
#endif // __ATOMIC__
      CurrentCacheLine = 0;
    }//1
    //If enough contiguous cache lines remain to hold the message, write the message (and metadata) to those lines as follows:
    //add the number of lines needed to the Reserve Sequence, and write the field with at least store-release semantics (if possible, if not, use full volatile sequential-consistency semantics)
    ReserveSequenceValue += FramingMetaDataTotalLines;
#ifdef __ATOMIC__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicReserveSequencePtr->store(ReserveSequenceValue, std::memory_order_release);
#else
    AtomicReserveSequencePtr->store(hton64(ReserveSequenceValue), std::memory_order_release);
#endif
#else
    WriteReserveSequence((void *)SharedMemoryBufferPtr, ReserveSequenceValue);
#endif
    FramingMetaDataTotalLinesPtr = (uint16_t *)((uint8_t *)SharedMemoryBufferPtr + __SharedMemoryDataPosition__ + (CurrentCacheLine * __CACHE_LINE_SIZE__));
    FramingMetaDataTotalNumberOfMessageBytesPtr = (uint16_t *)((uint8_t *)FramingMetaDataTotalLinesPtr + sizeof(FramingMetaDataTotalLines));
    FramingMetaDataMessageSequenceNumberPtr  = (uint64_t *)((uint8_t *)FramingMetaDataTotalNumberOfMessageBytesPtr + sizeof(FramingMetaDataTotalNumberOfMessageBytes));
    FramingMetaDataMessagePayloadPtr  = (void  *)((uint8_t *)FramingMetaDataMessageSequenceNumberPtr + sizeof(FramingMetaDataMessageSequenceNumberPtr));
    //write the meta data and payload bytes of the message at the current CacheLine
    *FramingMetaDataTotalLinesPtr = htons(FramingMetaDataTotalLines);
    *FramingMetaDataTotalNumberOfMessageBytesPtr = htons(FramingMetaDataTotalNumberOfMessageBytes);
    *FramingMetaDataMessageSequenceNumberPtr = hton64(FramingMetaDataMessageSequenceNumber);
    memcpy(FramingMetaDataMessagePayloadPtr, (void *)ReceivedDataBuffer, FramingMetaDataTotalNumberOfMessageBytes);
    //add the number of lines needed to the Commit Sequence.
    CommitSequenceValue += FramingMetaDataTotalLines;
    //At the end of the current event loop tick, write the Max Sequence Number and the Commit Sequence into the Control Header with sequential consistency semantics.
    MaxSequenceValue = FramingMetaDataMessageSequenceNumber;
    //This value is NOT committed to the Control Block yet, and instead will be written at “flush” time, which is normally at the end of the current event loop tick. It will be written with full volatile sequential-consistency semantics
#ifdef __ATOMIC__
#ifdef __SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicCommitSequencePtr->store(CommitSequenceValue, std::memory_order_seq_cst);
#else // __WRITE_LITTLE_ENDIAN__
    AtomicCommitSequencePtr->store(hton64(CommitSequenceValue), std::memory_order_seq_cst);
#endif //__WRITE_LITTLE_ENDIAN__
#else //__SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicCommitSequencePtr->store(CommitSequenceValue, std::memory_order_release);
#else //__WRITE_LITTLE_ENDIAN__
    AtomicCommitSequencePtr->store(hton64(CommitSequenceValue), std::memory_order_release);
#endif //__WRITE_LITTLE_ENDIAN__
#endif //__SEQ_CST__
#else //__ATOMIC__
    WriteCommitSequence((void *)SharedMemoryBufferPtr, CommitSequenceValue);
#endif //__ATOMIC__
    ShowInserterData();
    ShowInserterPointers();
    TimingStatisticsCollection();
  }//0
}

//////////////////////////////////////////////////////////////////////////////////////





int InserterImplementation::Initialize(CommonData *CommonDataPtr, Logger *LoggerPtr, Percentile *PercentilePtr,Tsc *TscPtr, RingFIFOBuffer *RingBufferPtr) {//-3
  ReturnValue = 0;

  this->LoggerPtr = LoggerPtr;
  this->PercentilePtr = PercentilePtr;
  this->TscPtr = TscPtr;
  this->RingBufferPtr = RingBufferPtr;
  if (!CommonDataPtr || !RingBufferPtr) {
    ReturnValue = -1;
  } 
  this->CommonDataPtr = CommonDataPtr;
  SharedMemoryFileDataOffset = __BLOCK_PAGE_SIZE__;
  this->WriterFlags = 0;
  WriterFlagsPrevious = 0;
  CommitSequenceValue = 0;
  ReserveSequenceValue = 0;
  SharedMemoryFileId = 0;
  FramingMetaDataTotalNumberOfMessageBytesPtr = 0;
  FramingMetaDataMessageSequenceNumberPtr = 0;
  FramingMetaDataMessagePayloadPtr = 0;
  SharedMemoryAllocationSize = __BLOCK_PAGE_SIZE__ * (1 + __NUMBER_OF_DATA_PAGES__);
  if (0 == ReturnValue) ReturnValue = OpenSharedMemoryFile(&SharedMemoryFileId, &SharedMemoryBufferPtr, SharedMemoryAllocationSize, CommonDataPtr->SharedMemoryName);
  if (ReturnValue == 0) {//-2
#ifdef __ATOMIC__
    SharedMemoryWriterFlagsPtr          = (uint8_t *)SharedMemoryBufferPtr + __WriterFlagsPosition__;
    AtomicWriterFlagsPtr    = new(SharedMemoryWriterFlagsPtr) std::atomic<uint32_t>;
    SharedMemoryWriterSessionIdPtr          = (uint8_t *)SharedMemoryBufferPtr + __WriterSessionIdPosition__;
    AtomicWriterSessionIdPtr  = new(SharedMemoryWriterSessionIdPtr)  std::atomic<uint64_t>;
    MaxSequencePtr      = (uint8_t *)SharedMemoryBufferPtr + __WriterMaxSequencePosition__;
    AtomicMaxSequencePtr    = new(MaxSequencePtr) std::atomic<uint64_t>;
    SharedMemoryCommitSequencePtr          = (uint8_t *)SharedMemoryBufferPtr + __CommitSequencePosition__;
    AtomicCommitSequencePtr        = new(SharedMemoryCommitSequencePtr)  std::atomic<uint64_t>;
    SharedMemoryReserveSequencePtr          = (uint8_t *)SharedMemoryBufferPtr + __ReserveSequencePosition__;
    AtomicReserveSequencePtr        = new(SharedMemoryReserveSequencePtr)  std::atomic<uint64_t>;
#endif

    CommonDataPtr->TotalNumberOfReceivedMessages = 0;
    CommonDataPtr->TotalNumberOfParsedFrames = 0;
    CommonDataPtr->TotalNumberOfRejectedMessages = 0;
    CommonDataPtr->TotalNumberOfParsedMessages = 0;
    CommonDataPtr->TotalNumberOfRejectedMessages = 0;
    NumberOfInsertedFramesSinceProgramStart = 0;
    SecondsSinceProgramStart = 0;

    WriteFileMarkerToSharedMemory();
    WriteFileVersion((int32_t)SharedMemoryFileVersion);
    WriteLayoutFlags((int32_t)SharedMemoryLayoutFlags);
    WritePageSize((uint32_t)(__BLOCK_PAGE_SIZE__));
    WriteLineSize((uint32_t)__CACHE_LINE_SIZE__);
    WriteDataSize((uint32_t)__SHARED_MEMORY_TOTAL_DATA_SIZE__);
    WriteDataOffset((uint64_t)SharedMemoryFileDataOffset);              // ?
    WriteWriterSessionId(CommonDataPtr->CommonValueSessionId);
    WriteWriterMaxSequence(MaxSequenceValue);
    WriteCommitSequence(CommitSequenceValue);
    WriteReserveSequence(ReserveSequenceValue);
    WriteWriterFlags(WriterFlags);

    LogInfo("file_marker\t\t=\t%s\t@\t%llu\n", ReadFileMarkerFromSharedMemory(), __FileMarkerPosition__);
    LogInfo("SharedMemoryFileVersion\t\t=\t%d\t@\t%llu\n", ReadFileVersion(), __FileVersionPosition__);
    LogInfo("SharedMemoryLayoutFlags\t\t=\t%d\t@\t%llu\n", ReadLayoutFlags(), __LayoutFlagsPosition__);
    LogInfo("page_size(lns)\t\t=\t%u\t@\t%llu\n", ReadPageSize(), __PageSizePosition__);
    LogInfo("line_size(B)\t\t=\t%u\t@\t%llu\n", ReadLineSize(), __LineSizePosition__);
    LogInfo("data_size(B)\t\t=\t%u\t@\t%llu\n", ReadDataSize(), __DataSizePosition__);
    LogInfo("SharedMemoryFileDataOffset(B)\t\t=\t%lu\t@\t%llu\n", ReadDataOffset(), __DataOffsetPosition__);    // in what units ?
    LogInfo("SessionId\t\t=\t%lu\t@\t%llu\n", ReadWriterSessionId(), __WriterSessionIdPosition__);
    LogInfo("MaxSequenceValue\t\t=\t%lu\t@\t%llu\n", ReadWriterMaxSequence(), __WriterMaxSequencePosition__);
    LogInfo("CommitSequenceValue\t\t=\t%lu\t@\t%llu\n", ReadCommitSequence(), __CommitSequencePosition__);
    LogInfo("ReserveSequenceValue\t=\t%lu\t@\t%llu\n", ReadReserveSequence(), __ReserveSequencePosition__);
    LogInfo("data_posution\t=\t%llu\n", __SharedMemoryDataPosition__);
    LogInfo("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ReadWriterFlags()), __WriterFlagsPosition__);

    if (CommonDataPtr) CommonDataPtr->ApplicationRunFlag = 1;
  }//-2
  else {//-2
    if (CommonDataPtr) CommonDataPtr->ApplicationRunFlag = 0;
  }//-2
//#ifdef __CONTROLABLE__
  if (CommonDataPtr) CommonDataPtr->ConfigurationFlags = 0;
//#endif // __CONTROLABLE__
  FramingMetaDataMessageSequenceNumber = 0;    //Sequence number of the message (int64)
#ifndef __TEST_PUBLISHER_ONLY__
  if (CommonDataPtr && ReturnValue == 0) 
#endif
  {//-2
    LogDebug("\n********malloc for %d bytes\n\n", __DATA_BUFFER_SIZE__);
    ReceivedDataPtr = (uint8_t *) new uint8_t[__DATA_BUFFER_SIZE__];
    if(ReceivedDataPtr) {
      memset(ReceivedDataPtr, 0, __DATA_BUFFER_SIZE__);
    }
    if (!ReceivedDataPtr) {//-1
      CommonDataPtr->ApplicationRunFlag = 0;
      LogError("circular buffer allocation error");
    }//-1
  }//-2
  if (CommonDataPtr && 0 == ReturnValue) {//-2
    if (0 == CommonDataPtr->ApplicationRunFlag) {//-1
      LogDebug("Started\n");
    }//-1
  }//-2
  return ReturnValue;
}//-3


void InserterImplementation::Iterate(void) {
   // while (CommonDataPtr->ApplicationRunFlag && CommonDataPtr->CommonFlagConnected)
    ReceivedDataBuffer = NULL;
    FramingMetaDataTotalNumberOfMessageBytes = 0;  //Total number of bytes in the message itself (uint16)
    unsigned int  NumberOfPayloadBytes = 0;

#ifdef __CONTROLABLE__
    uint64_t  timestamp = 0;
#endif //__CONTROLABLE__
    NumberOfPayloadBytes = RingBufferPtr->Read(ReceivedDataPtr);
    if (NumberOfPayloadBytes > 2 && ReceivedDataPtr != NULL) {//0
#ifdef __CONTROLABLE__
      if (TscPtr && (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_)) {//1
        //wiggle a timestamp if present
        if (NumberOfPayloadBytes == (ntohs(*(uint16_t*)ReceivedDataPtr) + sizeof(uint64_t) + 2)) {//2
          NumberOfPayloadBytes -= sizeof(uint64_t);
          timestamp = *(uint64_t*)&ReceivedDataPtr[NumberOfPayloadBytes];
          TscPtr->SetTimestamp(timestamp);
        }//2
      }//1
#endif //__CONTROLABLE__
      ReceivedDataBuffer = ReceivedDataPtr + 2; //exclude message size field, will be present in the frame header
      FramingMetaDataTotalNumberOfMessageBytes = NumberOfPayloadBytes - 2; ///exclude message size field, will be present in the frame header
      //clear condition and unlock mutex
      NumberOfPayloadBytes = 0;
    }//0
    //process changes in writer flags in established session
    UpdateWriterFlags();
    //process messages of an established session
    HandleSharedMemoryFrames();//AtomicReserveSequencePtr, AtomicCommitSequencePtr);
    ///////////// loop functions or state machine  ///////////
    WriteTimestampToSharedMemory();
}//-3



int InserterImplementation::Uninitialize(void) {
  uint32_t WriterFlags;
  if (ReceivedDataPtr) {
//#ifdef __CONTROLABLE__
    PrintSharedMemoryBlockPositions();
//#endif // __CONTROLABLE__
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ReadWriterFlags()), __WriterFlagsPosition__);
    WriterFlags = __WriterFlagsShutdown__;
#ifdef __ATOMIC__
#ifdef __SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(WriterFlags, std::memory_order_seq_cst);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(AtomicWriterFlagsPtr->load(std::memory_order_acquire)), __WriterFlagsPosition__);
#else // __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(htonl(WriterFlags), std::memory_order_seq_cst);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ntohl(AtomicWriterFlagsPtr->load(std::memory_order_acquire))), __WriterFlagsPosition__);
#endif // __WRITE_LITTLE_ENDIAN__
#else //__SEQ_CST__
#ifdef __WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(WriterFlags, std::memory_order_release);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(AtomicWriterFlagsPtr->load(std::memory_order_acquire)), __WriterFlagsPosition__);
#else //__WRITE_LITTLE_ENDIAN__
    AtomicWriterFlagsPtr->store(htonl(WriterFlags), std::memory_order_release);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ntohl(AtomicWriterFlagsPtr->load(std::memory_order_acquire))), __WriterFlagsPosition__);
#endif // //__WRITE_LITTLE_ENDIAN__
#endif //__SEQ_CST__
#else // __ATOMIC__
    WriteWriterFlags(SharedMemoryBufferPtr, WriterFlags);
    LogDebug("WriterFlags\t\t=\t%s\t@\t%llu\n", ParseWriterFlags(ReadWriterFlags(SharedMemoryBufferPtr)), __WriterFlagsPosition__);
#endif // __ATOMIC__
    PrintSessionIdFromSharedMemory();
  //logger.CloseLog();
    delete [] ReceivedDataPtr;
    ReceivedDataPtr = (uint8_t *) NULL;
    CloseSharedMemoryFile();
    return 0;
  } else {
    return -1;
  }
}



/////////////////////////////////////// InserterImplementation class constructor////////////////////////////////

InserterImplementation::InserterImplementation() {
  //SharedMemoryFileVersion = 1;
  //SharedMemoryLayoutFlags = 0;  //reserved for future use, now always 0.
  LoggerPtr = (Logger *) NULL;;
  PercentilePtr = (Percentile *) NULL;
  TscPtr = (Tsc *) NULL;
  CommonDataPtr = (CommonData *) NULL;
  ReceivedDataPtr = (uint8_t *) NULL;
  RingBufferPtr = (RingFIFOBuffer *) NULL;
}


InserterImplementation::~InserterImplementation() {
  Uninitialize();
}


int InserterImplementation::DoChangeFileOwner(const char *FilePathName, const char *UserName, const char *GroupName) {
  int rv = 0;
  uid_t          uid;
  gid_t          gid;
  struct passwd *pwd;
  struct group  *grp;
  if(UserName == NULL) {
    uid = -1;
  } else {
    pwd = getpwnam(UserName);
    if (pwd == NULL) {
        rv = -1;
        LogFprintfWarning("Failed to get uid");
    }
    uid = pwd->pw_uid;
  }
  grp = getgrnam(GroupName);
  if (grp == NULL) {
      rv = -2;
      LogFprintfWarning("Failed to get gid");
  }
  gid = grp->gr_gid;
  if (chown(FilePathName, uid, gid) == -1) {
      rv = -3;
      LogFprintfWarning("chown fail");
  }
  return rv;
}

// Function to reverse the byte order
template <typename T> T InserterImplementation::ReverseBytes(T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    T result = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        result |= (value & 0xFF) << ((sizeof(T) - 1 - i) * 8);
        value >>= 8;
    }
    return result;
}

// Host to Network (hton)
template <typename T> T InserterImplementation::HostToNetwork(T value) {
    return ReverseBytes<T>(value);
}

// Network to Host (ntoh)
template <typename T> T InserterImplementation::NetworkToHost(T value) {
    return ReverseBytes<T>(value);
}

#if(0)
#ifdef __WRITE_LITTLE_ENDIAN__
// Function to read a value from a memory pointer
template <typename T> T InserterImplementation::ReadMemory(void* ptr, size_t NumberOfOffsetBytes) {
    if (ptr != nullptr) {
      return *(T *)((uint8_t *)ptr + NumberOfOffsetBytes);
    }
    throw std::runtime_error("ReadMemory: Null pointer exception");
}
// Function for writing to a shared memory
template <typename T> void InserterImplementation::WriteMemory(void* ptr, size_t NumberOfOffsetBytes, T Value) {
    if (ptr != nullptr) {
      *(T *)((uint8_t *)ptr + NumberOfOffsetBytes) = Value;
      return;
    }
    throw std::runtime_error("WriteMemory: Null pointer exception");
}

#else
// Function to read a value from a memory pointera
template <typename T> T InserterImplementation::ReadMemory(void* ptr, size_t NumberOfOffsetBytes) {
    if (ptr != nullptr) {
      return ntoh<T>(*(T *)((uint8_t *)ptr + NumberOfOffsetBytes));
    }
    throw std::runtime_error("Null pointer exception");
}
// Function for writing to a shared memory
template <typename T> void InserterImplementation::WriteMemory(void* ptr, size_t NumberOfOffsetBytes, T Value) {
    if (ptr != nullptr) {
      *(T *)((uint8_t *)ptr + NumberOfOffsetBytes) = hton<T>(Value);
      return;
    }
    throw std::runtime_error("WriteMemory: Null pointer exception");
}

#endif
#else //(0)
#ifdef __WRITE_LITTLE_ENDIAN__
// Function to read a value from a memory pointer
template <typename T> T InserterImplementation::ReadMemory(size_t NumberOfOffsetBytes) {
    if (SharedMemoryBufferPtr != nullptr) {
      return *(T *)((uint8_t *)SharedMemoryBufferPtr + NumberOfOffsetBytes);
    } else {
      return (T) 0;
    }
}
// Function for writing to a shared memory
template <typename T> void InserterImplementation::WriteMemory(size_t NumberOfOffsetBytes, T Value) {
    if (SharedMemoryBufferPtr != nullptr) {
      *(T *)((uint8_t *)SharedMemoryBufferPtr + NumberOfOffsetBytes) = Value;
    }
}

#else
// Function to read a value from a memory pointera
template <typename T> T InserterImplementation::ReadMemory(size_t NumberOfOffsetBytes) {
    if (SharedMemoryBufferPtr != nullptr) {
      return NetworkToHost<T>(*(T *)((uint8_t *)SharedMemoryBufferPtr + NumberOfOffsetBytes));
    } else {
      return (T) 0;
    }
}
// Function for writing to a shared memory
template <typename T> void InserterImplementation::WriteMemory(size_t NumberOfOffsetBytes, T Value) {
    if (SharedMemoryBufferPtr != nullptr) {
      *(T *)((uint8_t *)SharedMemoryBufferPtr + NumberOfOffsetBytes) = HostToNetwork<T>(Value);
    }
}

#endif
#endif //(0)


#ifdef __SYSTEM_V__
int InserterImplementation::OpenSharedMemoryFile(int *SharedMemoryFileId, void **SharedMemoryBufferPtr, size_t SharedMemoryAllocationSize, char *SharedMemoryName) {
  int ReturnCode = 0;
  LogInfo("SharedMemoryAllocationSize=%ld, DataBlockPageSize=%lld\n", SharedMemoryAllocationSize, __BLOCK_PAGE_SIZE__);

  //SharedMemoryAllocationSize = ROUND_UP(SharedMemoryAllocationSize, __HUGE_PAGE_SIZE__);i
  SharedMemoryAllocationSize = ROUND_UP(SharedMemoryAllocationSize,__BLOCK_PAGE_SIZE__); 

  /* allocate shared memory */
  *SharedMemoryFileId = shmget(ftok(SharedMemoryName, 'R'), SharedMemoryAllocationSize, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
  if (*SharedMemoryFileId < 0) {
    LogWarn("shmget() failed. Possibile reasons include:\n"
      "- not enough huge pages available (e.g. configure vm.nr_hugepages)\n"
      "- SHM already exists due to unclean exit (check via 'ipcs', remove with 'ipcrm -n <page id>')\n"
       "- another controller already running\n");
    //TEST(0);
    ReturnCode = -1;
  } else {
    *SharedMemoryBufferPtr = shmat(*SharedMemoryFileId, NULL, 0);
    if (*SharedMemoryBufferPtr == (char *)(-1)) {
      LogWarn("shmat() failed.\n");
      //TEST(0);
      ReturnCode = -2;
    }
  }
  return ReturnCode;
}

void InserterImplementation::CloseSharedMemoryFile(void) {
  LOGV("About to exit, closing shared memory region\n");
  if (0 == shmdt(*SharedMemoryBufferPtr)) {
    shmctl(*SharedMemoryFileId, IPC_RMID, NULL);
  }
}

#else // POSIX

int InserterImplementation::OpenSharedMemoryFile(int *SharedMemoryFileId, void **SharedMemoryBufferPtr, size_t SharedMemoryAllocationSize, char *SharedMemoryName) {
  int ReturnCode = 0;
        LogInfo("SharedMemoryAllocationSize=%ld, HugePageSize=%lld\n", SharedMemoryAllocationSize, __BLOCK_PAGE_SIZE__);
        SharedMemoryAllocationSize = ROUND_UP(SharedMemoryAllocationSize, __BLOCK_PAGE_SIZE__);
        /* allocate shared memory */
#ifdef __USE_SHM__
        *SharedMemoryFileId = shm_open(SharedMemoryName, O_CREAT | O_TRUNC | O_RDWR, 0775);
#else
        *SharedMemoryFileId = open(SharedMemoryName, O_CREAT | O_TRUNC | O_RDWR, 0775);
#endif
  if (*SharedMemoryFileId == -1)
  {
    LogWarn("shm_open() failed.\n");
    //TEST(0);
    ReturnCode = -1;
  }
  else
  {
#ifdef __DO_CHOWN__
    if (DoChangeFileOwner (SharedMemoryName,NULL,"developers"))
    {
      close(*SharedMemoryFileId);
      LogWarn("chown() change group failed\n");
                        //TEST(0);
      ReturnCode = -2;
    }
    else {
#endif
      ReturnCode = ftruncate(*SharedMemoryFileId, SharedMemoryAllocationSize);
      LogInfo("\ftruncate() to file size %ld. Returns %d\n", SharedMemoryAllocationSize, ReturnCode);
      if(ReturnCode == 0) {
        *SharedMemoryBufferPtr = mmap(0, SharedMemoryAllocationSize, PROT_READ | PROT_WRITE, MAP_SHARED, *SharedMemoryFileId, 0);
        if (*SharedMemoryBufferPtr == MAP_FAILED) {
          LogWarn("mmap() failed\n");
          //TEST(0);
          ReturnCode = -3;
        } else {
          close(*SharedMemoryFileId);
        }
      }
    }
  }
  return ReturnCode;
}

void InserterImplementation::CloseSharedMemoryFile(void) {
  LOGV("About to exit, closing shared memory region\n");
}
#endif

char* InserterImplementation::ParseWriterFlags(uint32_t v) {
  static char f[128];
  memset((void *)f, 0, 128);
  if (v & __WriterFlagsConnected__)  strcat(f, "connected ");
  if (v & __WriterFlagsInSession__) strcat(f, "in_session ");
  if (v & __WriterFlagsShutdown__)   strcat(f, "shutdown ");
  if (v & __WriterFlagsLagged__)     strcat(f, "lagged");
  if (strlen(f) == 0)              strcpy(f, "none");
  return f;
}

//#ifdef __CONTROLABLE__
void InserterImplementation::PrintSharedMemoryBlockPositions(void) {
  PrintSessionIdFromSharedMemory();
  PrintFileControlBlockPositions();
  PrintDataSectionPositions();
}

void InserterImplementation::PrintFileControlBlockPositions(void) {
  PrintFileConfigurationInformationPositions();
  PrintWriterStateBlockPositions();
  PrintCurrentMessagePositions();
}

//      file control block

void InserterImplementation::PrintFileConfigurationInformationPositions(void) {
  LogFprintfInfo("file_configuration_information_positions - starting at 0:\n");
  PrintUnsigned32BitValue(__FileMarkerPosition__)
  PrintUnsigned32BitValue(__FileVersionPosition__)
  PrintUnsigned32BitValue(__LayoutFlagsPosition__)
  PrintUnsigned32BitValue(__PageSizePosition__)
  PrintUnsigned32BitValue(__LineSizePosition__)
  PrintUnsigned32BitValue(__DataSizePosition__)
  PrintUnsigned32BitValue(__DataOffsetPosition__)
}

void InserterImplementation::PrintWriterStateBlockPositions(void) {
  long long unsigned CurrentCacheLineNumber = (long long unsigned)(ntoh64(*(uint64_t *)((uint8_t *)SharedMemoryBufferPtr + __CommitSequencePosition__)) % __TOTAL_NUMBER_OF_DATA_CACHE_LINES__);
  //long long unsigned g_CurrentCacheLineNumber = (long long unsigned)g_CurrentCacheLine;
  LogFprintfInfo("writer_state_information positions - cache line aligned:\n");
  PrintUnsigned64BitValue(__WriterHeartbeatTimePosition__)
  PrintUnsigned64BitValue(__WriterSessionIdPosition__)
  PrintUnsigned64BitValue(__WriterMaxSequencePosition__)
  PrintUnsigned32BitValue(__WriterFlagsPosition__)
  PrintUnsigned64BitValue(__CommitSequencePosition__)
  PrintUnsigned64BitValue(__ReserveSequencePosition__)
  PrintUnsigned64BitValue(CurrentCacheLineNumber)
}

void InserterImplementation::PrintDataSectionPositions(void) {
  LogFprintfInfo("data section positions -  page aligned\n");
  PrintUnsigned64BitValue(__SharedMemoryDataPosition__)
  PrintUnsigned32BitValue(__SHARED_MEMORY_TOTAL_DATA_SIZE__)
}


void InserterImplementation::PrintCurrentMessagePositions(void) {
  if(FramingMetaDataTotalNumberOfMessageBytesPtr) {
    LogFprintfInfo("message:\n");
    Print16BitValueWithAddress(FramingMetaDataTotalLinesPtr)
    Print16BitValueWithAddress(FramingMetaDataTotalNumberOfMessageBytesPtr)
    Print64BitValueWithAddress(FramingMetaDataMessageSequenceNumberPtr)
    uint8_t *PayloadStartPtr = (uint8_t *)FramingMetaDataMessagePayloadPtr;
    uint8_t *PayloadEndPtr = (uint8_t *)(PayloadStartPtr -1 + ntohs((uint16_t)(*FramingMetaDataTotalNumberOfMessageBytesPtr)));
    Print8BitValueWithAddress(PayloadStartPtr)
    Print8BitValueWithAddress(PayloadEndPtr)
    LogFprintfInfo("message payload size = %u\n", ntohs((uint16_t)(*FramingMetaDataTotalNumberOfMessageBytesPtr)));
    //LogFprintfInfo("received data from parser (first 64 bytes):\n");
    //void *g_ReceivedDataBuffer = CommonDataPtr->ReceivedDataPtr + 2;
    //for (int i=0; i< 64; i++)
    //{
    //  LogFprintfInfo("%d: %02X (%c)\n", i, g_ReceivedDataBuffer[i], g_ReceivedDataBuffer[i]);
    //}
    //LogFprintfInfo("removed frame content:\n");
    //uint8_t *pf = (uint8_t *)FramingMetaDataTotalLinesPtr;
    //for (uint8_t *pr = pf; pr < (uint8_t *)FramingMetaDataTotalLinesPtr; pr++)
    //{
    //  Print8BitValueWithAddress(pr);
    //}
    //LogFprintfInfo("kept frame content:\n");
    //int i = 0;
    //int n = ntohs(*(uint16_t *)((uint8_t *)pf + 2));
    //LogFprintfInfo("n = %u\n", n);
    //for (uint8_t *pk = (uint8_t *)FramingMetaDataTotalLinesPtr; (i < n) && (pk <= PayloadEndPtr); pk++)
    //for (uint8_t *pk = pf; /*(i < n) &&*/ (pk <= PayloadEndPtr); pk++)
    //{
    //  LogFprintfInfo("%04u%c ", i, i < 12 ? '#' : ':');
    //  i++;
    //  Print8BitValueWithAddress(pk);
    //}
    uint64_t AverageNumberOfFramesPerSecond =( SecondsSinceProgramStart > 0) ? (uint64_t)(CommonDataPtr->TotalNumberOfReceivedMessages/SecondsSinceProgramStart) : (uint64_t) 0;;
    Print64BitValue(CommonDataPtr->TotalNumberOfReceivedMessages);
    Print64BitValue(CommonDataPtr->TotalNumberOfParsedFrames);
    Print64BitValue(CommonDataPtr->TotalNumberOfRejectedMessages);
    Print64BitValue(CommonDataPtr->TotalNumberOfParsedMessages);
    Print64BitValue(NumberOfInsertedFramesSinceProgramStart);
    Print64BitValue(SecondsSinceProgramStart);
    Print64BitValue(AverageNumberOfFramesPerSecond);
  } else {
    LogFprintfInfo("No messages received yet\n");
  }
}

void InserterImplementation::SetWriterFlags(uint32_t WriterFlagsVal) {
  this->WriterFlags = WriterFlagsVal;
}
//#endif // __CONTROLABLE__
// _eof_


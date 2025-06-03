
#ifndef __NENX_SHM_PUBLISHER_H__
#define __NENX_SHM_PUBLISHER_H__
#ifdef __ATOMIC__
//#define __SEQ_CST__
#undef __SEQ_CST__
#include <new>
#include <atomic>
#endif
#include "RingFIFOBuffer.h"
#include "CommonData.h"

#ifndef __SEQUENCE_NUMBER_LOCAL__
#ifdef __SEQUENCE_NUMBER_QUEUE_SEQUENCE__
typedef struct StrippedMessageHeaderStruct_t {
  uint8_t  HeaderLength;
  uint8_t  HeaderVersion;
  uint16_t MessageId;
  uint32_t QueueSequence;
  uint64_t SourceId;
  uint64_t QueueId;
  uint64_t TimeStamp;
  uint8_t  payload;
} __attribute__((packed)) StrippedMessageHeaderS_t;
#endif
#endif
//#define __CACHE_LINE_SIZE__  (64)
#ifdef __HUGE_PAGES__
#define __DO_CHOWN__
#define __HUGE_PAGE_SIZE__    (1ll * 1024 * 1024 * 1024)
//#define __HUGE_PAGE_SIZE__    (2ll * 1024 * 1024 * 1024)
#define __BLOCK_PAGE_SIZE__    (2ll * 1024 * 1024)
//#define __SHM_NAME_LENGTH__    (256)
//#undef __USE_SHM__
#ifdef __USE_SHM__
//#define SHM_NAME "/dev/shm/ef_vi_2mb.mmap"
#else
//#define SHM_NAME "/dev/hugepages/ef_vi_2mb.mmap"
//#define SHM_NAME "/dev/hugepages/ef_vi_2mb0.mmap"
#endif
//#define __NUMBER_OF_DATA_PAGES__    (1024) - overflows in java tester code
#define __NUMBER_OF_DATA_PAGES__    (512)
#else // __HUGE_PAGES__
#define  __HUGE_PAGE_SIZE__    (2ll * 1024 * 1024)
#define __BLOCK_PAGE_SIZE__    (4ll * 1024)
//#undef __USE_SHM__
#ifdef __USE_SHM__
//#define SHM_NAME "ef_vi.mmap"
#else
//#define SHM_NAME "/tmp/mmap/ef_vi_shared.mmap"
#endif
#define __NUMBER_OF_DATA_PAGES__        (262144)
#endif
//#define __NUMBER_OF_DATA_PAGES__  (__HUGE_PAGE_SIZE__/__BLOCK_PAGE_SIZE__)
#define __TOTAL_NUMBER_OF_DATA_CACHE_LINES__  ((__BLOCK_PAGE_SIZE__) * (__NUMBER_OF_DATA_PAGES__) / (__CACHE_LINE_SIZE__))
// shared memory block
//  file control block
//    file_configuration_information
#define __FileMarkerPosition__  (0ll) // starting at first page
#define __FileMarkerSize__  (sizeof(uint32_t))
#define __FileVersionPosition__  (__FileMarkerPosition__ + __FileMarkerSize__)
#define __FileVersionSize__  (sizeof(int32_t))
#define __LayoutFlagsPosition__  (__FileVersionPosition__ + __FileVersionSize__)
#define __LayoutFlagsSize__  (sizeof(int32_t))
#define __PageSizePosition__  (__LayoutFlagsPosition__ + __LayoutFlagsSize__)
#define __PageSizeSize__    (sizeof(uint32_t))
#define __LineSizePosition__  (__PageSizePosition__ + __PageSizeSize__)
#define __LineSizeSize__    (sizeof(uint32_t))
#define __DataSizePosition__  (__LineSizePosition__ + __LineSizeSize__)
#define __DataSizeSize__    (sizeof(uint32_t))
#define __DataOffsetPosition__  (__DataSizePosition__ + __DataSizeSize__)
#define __DataOffsetSize__  (sizeof(uint32_t))
//    writer_state_information - cache line aligned
#define __WriterHeartbeatTimePosition__  (((__DataOffsetPosition__ + __DataOffsetSize__ + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__)
#define __WriterHeartbeatTimeSize__  (sizeof(int64_t))
#define __WriterSessionIdPosition__  (__WriterHeartbeatTimePosition__ + __WriterHeartbeatTimeSize__)
#define __WriterSessionIdSize__    (sizeof(uint64_t))
#define __WriterMaxSequencePosition__  (__WriterSessionIdPosition__ + __WriterSessionIdSize__)
#define __WriterMaxSequenceSize__  (sizeof(int64_t))
#define __WriterFlagsPosition__    (__WriterMaxSequencePosition__ + __WriterMaxSequenceSize__)
#define __WriterFlagsSize__    (sizeof(uint32_t))
enum __WriterFlags__ {
  __WriterFlagsConnected__  = 0x01,  // - writer is connected to its own data source
  __WriterFlagsInSession__  = 0x02, // - writer is currently in session
  __WriterFlagsShutdown__  = 0x04, // - writer’s current session has been shutdown (no further sequence numbers are being inserted)
  __WriterFlagsLagged__  = 0x08  // - writer’s data source is lagged
};

#define __CommitSequencePosition__  (((__WriterFlagsPosition__ + __WriterFlagsSize__ + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__) //cache line aligned
#define __CommitSequenceSize__    (sizeof(uint64_t))
#define __ReserveSequencePosition__  (((__CommitSequencePosition__ + __CommitSequenceSize__ + __CACHE_LINE_SIZE__ - 1) / __CACHE_LINE_SIZE__) * __CACHE_LINE_SIZE__) //cache line aligned
#define __ReserveSequenceSize__    (sizeof(uint64_t))
//  data section  = page aligned
#define __SharedMemoryDataPosition__      (((__ReserveSequencePosition__ + __ReserveSequenceSize__ + __BLOCK_PAGE_SIZE__ -1) / __BLOCK_PAGE_SIZE__) * __BLOCK_PAGE_SIZE__)
#define __SHARED_MEMORY_TOTAL_DATA_SIZE__      (__BLOCK_PAGE_SIZE__ * __NUMBER_OF_DATA_PAGES__)
#ifndef __x86_64__
  #error Current code for x86_64 only
#endif
//////////////////////////diagnostics////////////////////////////////////1145258561/////////////////////////////////////////////////
#define PrintUnsigned32BitValue(q) printf("\t%s\t= %llu\t(0x%08llX)\t[%016lX]\n",#q,q,q,ntoh64(*(uint64_t *)((uint8_t *)SharedMemoryBufferPtr + q)));
#define PrintUnsigned64BitValue(q) printf("\t%s\t= %llu\t(0x%016llX)\t[%016lX]\n",#q,q,q,ntoh64(*(uint64_t *)((uint8_t *)SharedMemoryBufferPtr + q)));


#ifdef __USE_DEFINES_FOR_SHARED_MEMORY_ACCESS__

#define WriteFileVersion(v) WriteMemory<int32_t>((__FileVersionPosition__), (v))
#define ReadFileVersion() ReadMemory<uint32_t>((__FileVersionPosition__))
#define WriteLayoutFlags(v) WriteMemory<int32_t>((__LayoutFlagsPosition__), (v))
#define ReadLayoutFlags() ReadMemory<uint32_t>((__LayoutFlagsPosition__))
#define WritePageSize(v) WriteMemory<uint32_t>((__PageSizePosition__), (v))
#define ReadPageSize() ReadMemory<uint32_t>((__PageSizePosition__))
#define WriteLineSize(v) WriteMemory<uint32_t>((__LineSizePosition__), (v))
#define ReadLineSize() ReadMemory<uint32_t>((__LineSizePosition__))
#define WriteDataSize(v) WriteMemory<uint32_t>((__DataSizePosition__), (v))
#define ReadDataSize() ReadMemory<uint32_t>((__DataSizePosition__))
#define WriteDataOffset(v) WriteMemory<uint64_t>((__DataOffsetPosition__), (v))
#define ReadDataOffset() ReadMemory<uint64_t>((__DataOffsetPosition__))
//              writer_state_information - cache line aligned
#define WriteWriterHeartbeatTime(v) WriteMemory<uint64_t>((__WriterHeartbeatTimePosition__), (v))
#define ReadWriterHeartbeatTime() ReadMemory<uint64_t>((__WriterHeartbeatTimePosition__))
#define WriteWriterSessionId(v) WriteMemory<uint64_t>((__WriterSessionIdPosition__), ntoh64(v))
#define ReadWriterSessionId() hton64(ReadMemory<uint64_t>((__WriterSessionIdPosition__)))
#define WriteWriterMaxSequence(v) WriteMemory<uint64_t>((__WriterMaxSequencePosition__), (v))
#define ReadWriterMaxSequence() ReadMemory<uint64_t>((__WriterMaxSequencePosition__))
#define WriteWriterFlags(v) WriteMemory<uint32_t>((__WriterFlagsPosition__), (v))
#define ReadWriterFlags() ReadMemory<uint32_t>((__WriterFlagsPosition__))
#define WriteCommitSequence(v) WriteMemory<uint64_t>((__CommitSequencePosition__), (v))
#define ReadCommitSequence() ReadMemory<uint64_t>((__CommitSequencePosition__))
#define WriteReserveSequence(v) WriteMemory<uint64_t>((__ReserveSequencePosition__), (v))
#define ReadReserveSequence() ReadMemory<uint64_t>((__ReserveSequencePosition__))

#endif //__USE_DEFINES_FOR_SHARED_MEMORY_ACCESS__

#endif // __NENX_SHM_PUBLISHER_H__
// eof

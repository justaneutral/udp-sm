#ifndef __PARSER_H__
#define __PARSER_H__

#include "defines.h"
#include "flags.h"
#include "utils.h"
#include "CommonData.h"
#include "Logger.h"
#include "RingFIFOBuffer.h"
#include "ParserInterface.h"

#ifdef __DEBUG_INJECT_SESSION_ID__
#define __INJECT_SESSION_ID__ \
  *(uint8_t*)&FrameHeaderPtr->SessionIdVal = 'a'; \
  *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 1) = 'b'; \
  *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 2) = 'c'; \
  *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 3) = 'd'; \
  *(uint32_t*)((uint8_t*)&FrameHeaderPtr->SessionIdVal + 4) = htonl((uint32_t)12345678);
#else //__INJECT_SESSION_ID__
#define __INJECT_SESSION_ID__
#endif // __INJECT_SESSION_ID__


typedef enum {
  ParserMessageHeartbeat = 0,
  ParserMessageShutdown,
  ParserMessageSequenced,
  ParserMessageUnsequenced = 0x64
} ParserMessageTypeEnum_t;

typedef enum {
  ParserStateStartup = 0,
  ParserStateSession,
  ParserStateshutdown
} ParserStateEnum_t;

typedef struct ParserStateStruct_t {
  uint64_t SessionIdVal;
  uint64_t SessionIdPrevious;
  int      state;
} ParserState_t;


typedef struct FrameHeaderStruct_t {
  uint8_t   MessageType;
  uint8_t   MessageHeaderLength;
  uint64_t  SessionIdVal;
  uint64_t  SequenceNumber;
  uint16_t  MessageCount;
} __attribute__((packed)) FrameHeaderS_t;

typedef union FrameHeaderUnion_t {
  FrameHeaderS_t  s;
  uint8_t b[20];
} __attribute__((packed)) FrameHeaderU_t;


typedef struct MessageHeaderStruct_t {
  uint16_t  MessageLength;
  uint8_t   HeaderLength;
  uint8_t   HeaderVersion;
  uint16_t  MessageId;
  uint32_t  QueueSequence;
  uint64_t  SourceId;
  uint64_t  QueueId;
  uint64_t  TimeStamp;
  uint8_t   payload;
} __attribute__((packed)) MessageHeaderS_t;

typedef union MessageHeaderUnion_t {
  MessageHeaderS_t s;
  uint8_t b[34];
} __attribute__((packed)) MessageHeaderU_t;


//void ParserPrintHeader(FrameHeaderS_t *FrameHeaderPtr, uint8_t discard, int LocalConfigurationFlags, size_t bytes);
//void ParserPrintMessage(int MessageIndex, uint32_t ml, MessageHeaderS_t MessageHeaderPtr, uint16_t mc, int ConfigurationFlags, int ConfigThreshold, uint8_t DiscardFrame, size_t PayloadBytesSent, size_t PayloadBytesReceived);


class ParserImplementation : public ParserInterface {
public:
  ParserImplementation(CommonData *CommonDataPtr, Logger *LoggerPtr, RingFIFOBuffer *RingBufferPtr0, RingFIFOBuffer *RingBufferPtr1);
  ~ParserImplementation(void);
  void Iterate(void);
  //void set_logger(void);
private:
  int IterateInternal(void);
  void UpdateParserState(void);
  void ParserPrintHeader(void);
  void ParserPrintMessage(void);
public:
  int Initialized;
private:
  Logger *LoggerPtr;
  CommonData *CommonDataPtr;
  uint8_t *ParserReceivedDataPtr;
  ParserState_t ParserStateInstance;
  FrameHeaderS_t  *FrameHeaderPtr;
  MessageHeaderS_t *MessageHeaderPtr;
  uint8_t DiscardFrame;
  size_t PayloadBytesReceived;
  uint32_t MessageLengthBytes;
  uint16_t MessageCounter;
  size_t PayloadBytesProcessed;
  int MessageIndex;
  RingFIFOBuffer *RingBufferPtr0;
  RingFIFOBuffer *RingBufferPtr1;
};

#endif //__PARSER_H_
// _eof_

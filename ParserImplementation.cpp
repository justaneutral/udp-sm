
#include "ParserImplementation.h"

ParserImplementation::ParserImplementation(CommonData *CommonDataPtr, Logger *LoggerPtr, RingFIFOBuffer *RingBufferPtr0, RingFIFOBuffer *RingBufferPtr1) {
  this->Initialized = 0;
  this->CommonDataPtr = CommonDataPtr;
  this->LoggerPtr = LoggerPtr;
  this->RingBufferPtr0  = RingBufferPtr0;
  this->RingBufferPtr1 = RingBufferPtr1;
  ParserStateInstance.SessionIdVal = 0;
  ParserStateInstance.SessionIdPrevious = 0;
  ParserStateInstance.state = ParserStateStartup;
  ParserReceivedDataPtr = (uint8_t *) NULL;
  if (this->CommonDataPtr) {
    this->CommonDataPtr->CommonValueSequenceNumber = 0;
    this->CommonDataPtr->TotalNumberOfParsedFrames = 0;
    this->CommonDataPtr->TotalNumberOfRejectedMessages = 0;
    this->CommonDataPtr->TotalNumberOfParsedMessages = 0;
    ParserReceivedDataPtr = (uint8_t *) new uint8_t[__DATA_BUFFER_SIZE__];
    //ParserReceivedDataPtr = (uint8_t *) malloc(__DATA_BUFFER_SIZE__);
    if (!ParserReceivedDataPtr) {
      //this->Initialized = 0;
      LogDebug("\n   =======> parser:=======> malloc error\n");
    } else {
      memset(this->ParserReceivedDataPtr, 0, __DATA_BUFFER_SIZE__);
      this->Initialized = 1; 
    }
  } else {
    //this->Initialized = 0;
    LogDebug("\n   =======> parser:=======>  common is NULL\n");
  }
}

ParserImplementation::~ParserImplementation(void) {
  if (Initialized) {
    if (ParserReceivedDataPtr) {
      //free(ParserReceivedDataPtr);
      delete [] ParserReceivedDataPtr;
      ParserReceivedDataPtr = NULL;
    }
    Initialized = 0;
  }
}


void ParserImplementation::Iterate(void) {
  //puchback a little at parser
  if(Initialized && CommonDataPtr && RingBufferPtr1 && (RingBufferPtr1->GetCapacity() >= 16384)) {//0
    if (IterateInternal()) {//1
      CommonDataPtr->ApplicationRunFlag = 0; //error in ring buffer - leave
    }//1
  }//0
}

int ParserImplementation::IterateInternal(void) {//0
  int rv = 0;
  DiscardFrame = 0;
  FrameHeaderPtr = NULL;
  MessageHeaderPtr  = NULL;
  MessageLengthBytes = 0;
  MessageCounter = 0;
  size_t PayloadBytesProcessed, PayloadBytesSent;
  if (CommonDataPtr->ApplicationRunFlag && CommonDataPtr->CommonFlagConnected) {//1
    //initialize frame processing
    DiscardFrame = 0x00;
    PayloadBytesReceived = RingBufferPtr0->Read(ParserReceivedDataPtr);
#ifdef __CONTROLABLE__
    if (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_) {
      if (PayloadBytesReceived >= (18 + sizeof(uint64_t))) {
        PayloadBytesReceived -= sizeof(uint64_t);
      }
    }
#endif //__CONTROLABLE__
    if (PayloadBytesReceived > 0) {//1.5
      if (PayloadBytesReceived >= 36) {//2
        //frame must have at least one payloadless message
//#ifdef __CONTROLABLE__
        CommonDataPtr->TotalNumberOfParsedFrames++;
//#endif //__CONTROLABLE__
        FrameHeaderPtr = (FrameHeaderS_t *)ParserReceivedDataPtr;
        if (FrameHeaderPtr->MessageHeaderLength != 18) {
          DiscardFrame = 0x01;
        }
        if (ntohs(FrameHeaderPtr->MessageCount) <= 0) {
          DiscardFrame |= 0x02;
        }
        if (!DiscardFrame) {//3
          __INJECT_SESSION_ID__
//#ifdef __CONTROLABLE__
          ParserPrintHeader();
//#endif //__CONTROLABLE__
        //update system state
          UpdateParserState();
          if (!DiscardFrame) {//4
            size_t MessageBytesReceived = PayloadBytesReceived - FrameHeaderPtr->MessageHeaderLength - 2;
            size_t MessageBytesExpected = 0;
            MessageHeaderPtr = (MessageHeaderS_t *)(&FrameHeaderPtr->MessageCount + (size_t)1);  //this works
            MessageCounter = ntohs(FrameHeaderPtr->MessageCount);
//#ifdef __CONTROLABLE__
            if ((CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_) &&  (MessageCounter > CommonDataPtr->ConfigThreshold)) {
              LogInfo("\n   =======> parser: =======> received %ld bytes\n", MessageBytesReceived);
            }
//#endif //__CONTROLABLE__
            PayloadBytesProcessed = 0;
            PayloadBytesSent = 0;
            if (MessageCounter > 0) {//5
              //remember the state of the buffer to be able to discard ther changes
              RingBufferPtr1->Snap();
              for (MessageIndex = 0; (MessageIndex < MessageCounter) && !DiscardFrame; MessageIndex++) {//6
                MessageLengthBytes = 2 + ntohs(MessageHeaderPtr->MessageLength);
                MessageBytesExpected += MessageLengthBytes;
                if ((int) MessageLengthBytes < (2 + MessageHeaderPtr->HeaderLength) || (MessageLengthBytes + 20 + PayloadBytesProcessed) > PayloadBytesReceived) {
                  DiscardFrame |= 0x04;
                }
                if (!((MessageHeaderPtr->HeaderLength == 32) || (MessageHeaderPtr->HeaderLength == 28))) {
                  DiscardFrame |= 0x08;
                }
                if (MessageHeaderPtr->HeaderVersion != 1) {
                  DiscardFrame |= 0x10;
                }
//#ifdef __CONTROLABLE__
                ParserPrintMessage();
//#endif // __CONTROLABLE__
                size_t CurrentlySentBytes;
                CurrentlySentBytes = 0;
#ifdef __CONTROLABLE__
                if ((!DiscardFrame) && (CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_) && (MessageIndex == (MessageCounter-1))) {
                  do {
                    CurrentlySentBytes = RingBufferPtr1->Write((unsigned char *)MessageHeaderPtr, (size_t)(MessageLengthBytes) + sizeof(uint64_t));
                  } while (CurrentlySentBytes == 0);
                } else
#endif //__CONTROLABLE__
                {
                  do {
                    CurrentlySentBytes = RingBufferPtr1->Write((unsigned char *)MessageHeaderPtr, (size_t)(MessageLengthBytes));
                  } while (CurrentlySentBytes == 0);
                }
                PayloadBytesSent += MessageLengthBytes;
//#ifdef __CONTROLABLE__
                if ((CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_) && (MessageCounter > CommonDataPtr->ConfigThreshold)) {
                  LogInfo("\nsent %ld bytes from %lu\n", PayloadBytesSent, (uint64_t)MessageHeaderPtr);
                }
//#endif //__CONTROLABLE__
                if (MessageIndex + 1 < MessageCounter) {
                  uint8_t  *MessageHeaderPtr0 = (uint8_t *)MessageHeaderPtr;
                  MessageHeaderPtr = (MessageHeaderS_t *)(MessageHeaderPtr0 + (size_t)MessageLengthBytes);
//#ifdef __CONTROLABLE__
                  if ((CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_) && (MessageCounter > CommonDataPtr->ConfigThreshold)) {
                    size_t jmp = (size_t)MessageHeaderPtr - (size_t)MessageHeaderPtr0;
                    LogInfo("next message at %lu, jump = %lu\n", (uint64_t)MessageHeaderPtr, jmp);
                  }
//#endif //__CONTROLABLE__
                } else {
                  if (MessageBytesExpected != MessageBytesReceived) {
                    DiscardFrame |= 0x20;
                  }
//#ifdef __CONTROLABLE__
                  if ((CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_) && (MessageCounter > CommonDataPtr->ConfigThreshold)) {
                    LogInfo("last message processed\n\n");
                  }
//#endif //__CONTROLABLE__
                }
              }//6
              if (DiscardFrame) {
                RingBufferPtr1->Discard();
              }
              else {
                CommonDataPtr->CommonValueSequenceNumber = ntoh64(FrameHeaderPtr->SequenceNumber);
//#ifdef __CONTROLABLE__
                if ((CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_) && (MessageCounter > CommonDataPtr->ConfigThreshold)) {
                  LogInfo("last message sent\n\n");
                }
//#endif //__CONTROLABLE__
              }
            }//5
          }//4
        }//3
      } else { //2
      //check for frame with no messages
        if (PayloadBytesReceived == 18) {//2
//#ifdef __CONTROLABLE__
          CommonDataPtr->TotalNumberOfParsedFrames++;
//#endif //__CONTROLABLE__
          FrameHeaderPtr = (FrameHeaderS_t *)ParserReceivedDataPtr;
          if (FrameHeaderPtr->MessageHeaderLength == 16) {
            __INJECT_SESSION_ID__
//#ifdef __CONTROLABLE__
            ParserPrintHeader();
//#endif //__CONTROLABLE__  
          }
        }//2
//#ifdef __CONTROLABLE__
        else {
          CommonDataPtr->TotalNumberOfRejectedMessages++;
        }
//#endif //__CONTROLABLE__
      }
    }//1.5
  }//1
  return rv;
}

void ParserImplementation::UpdateParserState(void) {
  ParserState_t *ParserStateLocalPtr = &ParserStateInstance;
  int *ShutdownReceivedPtr = &CommonDataPtr->CommonFlagShutdownReceived;
  uint64_t *SessionIdPtr =  &CommonDataPtr->CommonValueSessionId;
  int *ReceivedSessionIdPtr = &CommonDataPtr->CommonFlagSessionIdReceived;
  uint8_t *DiscardFramePtr = &DiscardFrame;
  RingFIFOBuffer* RingBufferLocalPtr = RingBufferPtr1;
//#ifdef __CONTROLABLE__
  int LocalConfigurationFlags = CommonDataPtr->ConfigurationFlags;
//#endif // __CONTROLABLE__

  switch (ParserStateLocalPtr->state) {
  case ParserStateStartup:
    if ((FrameHeaderPtr->MessageType != ParserMessageShutdown) /*&& (FrameHeaderPtr->SessionIdVal != (uint64_t)0)*/) {
      ParserStateLocalPtr->SessionIdPrevious = ParserStateLocalPtr->SessionIdVal;
      ParserStateLocalPtr->SessionIdVal = FrameHeaderPtr->SessionIdVal;
      ParserStateLocalPtr->state = ParserStateSession;
      *ShutdownReceivedPtr = 0;
      *SessionIdPtr = FrameHeaderPtr->SessionIdVal;
      *ReceivedSessionIdPtr = 1;
    }
    break;
  case ParserStateSession:
    if ((FrameHeaderPtr->SessionIdVal !=  ParserStateLocalPtr->SessionIdVal)) {
      LogDebug("\nWrong SessionIdVal %016lX, must be %016lX, message:", ntoh64(FrameHeaderPtr->SessionIdVal), ntoh64( ParserStateLocalPtr->SessionIdVal));
      for (unsigned int i = 0; i < PayloadBytesReceived; i++) {
        LogDebug(" %02X", *((unsigned char *)FrameHeaderPtr + (size_t)i));
      }
      LogDebug("\n");
//#ifdef __CONTROLABLE__
      if (!(LocalConfigurationFlags & _FLAG_DONT_FILTER_SOUCE_ID_)) {
        *DiscardFramePtr |= 0x20;
        LogDebug("message discarded by parser\n");
      }
//#endif // __CONTROLABLE__
    } else {
      if ((FrameHeaderPtr->MessageType == ParserMessageShutdown)) {
        ParserStateLocalPtr->state = ParserStateshutdown;
      }
    }
    break;
  case ParserStateshutdown:
    if (RingBufferLocalPtr->ringbuf.NumberOfStoredBytes == 0 || RingBufferLocalPtr->ringbuf.FramesStored == 0) { // all stored messages written to shared memory
      ParserStateLocalPtr->SessionIdPrevious = 0;
      ParserStateLocalPtr->SessionIdVal = 0;
      ParserStateLocalPtr->state = ParserStateStartup;
      *ShutdownReceivedPtr = 1;
      *ReceivedSessionIdPtr = 0;
    }
    break;
  default:
    *DiscardFramePtr |= 0x40;
  }
}


///// debug and test functions
//inline 
void ParserImplementation::ParserPrintHeader(void) {
  uint8_t discard = DiscardFrame; 
  int LocalConfigurationFlags = CommonDataPtr->ConfigurationFlags;
  size_t bytes = PayloadBytesReceived;

  if (LocalConfigurationFlags & _FLAG_SHOW_PARSER_) {
    LogFprintfInfo("\n   =======> main_parser_thread :=======> received %ld bytes\n", bytes);
    LogFprintfInfo("MessageType = 0x%02X\n", FrameHeaderPtr->MessageType);
    if (FrameHeaderPtr->MessageType == __MESSAGE_TYPE_SHUTDOWN__) {
      LogFprintfInfo("received message type __MESSAGE_TYPE_SHUTDOWN__\n");
    }
    LogFprintfInfo("MessageHeaderLength = %02u\n", FrameHeaderPtr->MessageHeaderLength);
    if (discard & 0x01) {
      LogFprintfInfo("wrong frame header length, frame discarded\n");
    }
    LogFprintfInfo("SessionIdVal = %c%c%c%c%08X\n", *(uint8_t*)&FrameHeaderPtr->SessionIdVal, *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 1), *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 2), *((uint8_t*)&FrameHeaderPtr->SessionIdVal + 3), ntohl(*(uint32_t*)((uint8_t*)&FrameHeaderPtr->SessionIdVal + 4)));
    LogFprintfInfo("SequenceNumber = %016lX\n", ntoh64(FrameHeaderPtr->SequenceNumber));

    switch (FrameHeaderPtr->MessageHeaderLength) {
    case 18: 
      LogFprintfInfo("MessageCount = %04u\n", ntohs(FrameHeaderPtr->MessageCount));
      if (discard & 0x02) {
        LogFprintfInfo("wrong message count, frame discarded\n");
      }
      break;
    case 16:
      LogFprintfInfo("messageless frame\n");
      break;
    default:
      LogFprintfInfo("error: this frame has incorrect frame length\n");
    }
  }
}



//inline 
void ParserImplementation::ParserPrintMessage(void) {
  int ConfigurationFlags = CommonDataPtr->ConfigurationFlags;
  int ConfigThreshold = CommonDataPtr->ConfigThreshold;
  size_t PayloadBytesSent = PayloadBytesProcessed;

  if ((ConfigurationFlags & _FLAG_SHOW_PARSER_) &&  (MessageCounter > ConfigThreshold)) {
    LogFprintfInfo("message # %d\n", MessageIndex);
    LogFprintfInfo("message length = %d\n", MessageLengthBytes);

    if (DiscardFrame & 0x04) {
      LogFprintfInfo(" -------- wrong message length ?\n");
      LogFprintfInfo("MessageLengthBytes = %d,  PayloadBytesSent = %ld,  PayloadBytesReceived = %ld\n", MessageLengthBytes, PayloadBytesSent, PayloadBytesReceived);
      fflush(stdout);
    }
                                
    LogFprintfInfo("header length = %d\n", MessageHeaderPtr->HeaderLength);
    if (DiscardFrame & 0x08) {
      LogFprintfInfo(" -------- wrong header length ?\n");
    }

    LogFprintfInfo("header version = %d\n", MessageHeaderPtr->HeaderVersion);
    if (DiscardFrame & 0x10) {
      LogFprintfInfo(" -------- wrong header version ?\n");
    }

    LogFprintfInfo("MessageId = %04X\n", ntohs(MessageHeaderPtr->MessageId));
    LogFprintfInfo("queue sequence = %08X\n", ntohl(MessageHeaderPtr->QueueSequence));
    LogFprintfInfo("source ID = %016lX\n", ntoh64((MessageHeaderPtr->SourceId)));
    LogFprintfInfo("Queue ID = %016lX\n", ntoh64((MessageHeaderPtr->QueueId)));

    if (MessageHeaderPtr->HeaderLength == 32) {
      LogFprintfInfo("Time Stamp = %016lX\n", ntoh64((MessageHeaderPtr->TimeStamp)));
    }

    int PayloadLength = MessageLengthBytes - MessageHeaderPtr->HeaderLength;
    LogFprintfInfo("Payload Length = %d\n", PayloadLength);
  }
}

// _eof_

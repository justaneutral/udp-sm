
#include "KeyboardReadInput.h"

int *RunFlagSignal = NULL;
void signal_handler(int ProgramInterruptSignalNumber) {
  if (ProgramInterruptSignalNumber == SIGINT) {
    *RunFlagSignal = 0;
  }
  assert(ProgramInterruptSignalNumber == SIGINT);
}

//#ifdef __CONTROLABLE__

KeyboardReadInput::KeyboardReadInput(CommonData *CommonDataPtr, InserterInterface * InserterInterfacePtr, Logger *LoggerPtr, Percentile *PercentilePtr, RingFIFOBuffer *RingBufferPtr0, RingFIFOBuffer *RingBufferPtr1) {
  this->CommonDataPtr = CommonDataPtr;
  this->InserterInterfacePtr = InserterInterfacePtr;
  this->LoggerPtr = LoggerPtr;
  this->PercentilePtr = PercentilePtr;
  this->RingBufferPtr0 = RingBufferPtr0;
  this->RingBufferPtr1 = RingBufferPtr1;
}

KeyboardReadInput::~KeyboardReadInput() {}

int KeyboardReadInput::KeyboardHit(void) {
  int BytesWaitingInKeyboardBuffer;
  ioctl(0, FIONREAD, &BytesWaitingInKeyboardBuffer);
  return BytesWaitingInKeyboardBuffer;
}


void KeyboardReadInput::ReadKeyboardInput(void) {
  if (KeyboardHit()) {
    char c = getchar();
    if (!CommonDataPtr) {
      LogFprintfInfo("WARNING: Common Data Pointer is NULL !");
    } else switch (c) {
    case 'v':
      if (CommonDataPtr->Verbose) {
        CommonDataPtr->Verbose = 0;
        LoggerPtr->SetLogVerbose(0);
      } else {
        CommonDataPtr->Verbose = 1;
        LoggerPtr->SetLogVerbose(1);
      }
      break;
    case 'b':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_HEARTBEAT_;
      break;
    case 'u':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_DEC_DUMP_;
      break;
    case 'x':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_HEX_DUMP_;
      break;
    case 'V':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_VERBOSE_;
      break;
    case 'p':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_PARSER_;
      break;
    case 'P':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_DONT_FILTER_SOUCE_ID_;
      break;
    case 'd':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_PUBLISHER_;
      break;
    case 'D':
      CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_DATA_;
      break;
#ifdef __CONTROLABLE__
    case 't':
      if (PercentilePtr) CommonDataPtr->ConfigurationFlags ^= _FLAG_SHOW_TIMING_;
      else LogFprintfInfo("Statistic modules Tsc or Percentile are not instantiated\n");
      break;
    case 'T':
      if (PercentilePtr) CommonDataPtr->ConfigurationFlags ^= _FLAG_GET_TIMING_;
      else LogFprintfInfo("Statistic modules Tsc or Percentile are not instantiated\n");
      break;
#endif // __CONTROLABLE__
    case '-':
      CommonDataPtr->ConfigThreshold = CommonDataPtr->ConfigThreshold > 0 ? CommonDataPtr->ConfigThreshold - 1 : 0;
      break;
    case '+':
      CommonDataPtr->ConfigThreshold = CommonDataPtr->ConfigThreshold >= 0 ? CommonDataPtr->ConfigThreshold + 1 : 0;
      break;
    case 's':
      CommonDataPtr->ConfigSleep = CommonDataPtr->ConfigSleep > 0 ? CommonDataPtr->ConfigSleep - 1 : 0;
      break;
    case 'S':
      CommonDataPtr->ConfigSleep = CommonDataPtr->ConfigSleep >= 0 ? CommonDataPtr->ConfigSleep + 1 : 0;
      break;
    case 'm':
      if (InserterInterfacePtr) {
        InserterInterfacePtr->PrintSharedMemoryBlockPositions();
      } else {
        LogFprintfInfo("WARNING: Inserter pointer is NULL !");
      }
      if (RingBufferPtr0) {
        RingBufferPtr0->PrintStatus("receiver to parser");
      } else {
        LogFprintfInfo("WARNING: Ring buffer 0 pointer is NULL !");
      }
      if (RingBufferPtr1) {
        RingBufferPtr0->PrintStatus("parser to inserter");
      } else {
        LogFprintfInfo("WARNING: Ring buffer 1 pointer is NULL !");
      }
      break;
#ifdef __CONTROLABLE__
    case '%':
      if (PercentilePtr) PercentilePtr->PrintPercentileTableValues();
      else LogFprintfInfo("Statistic modules Tsc or Percentile are not instantiated\n");
      break;
    case '#':
      if (PercentilePtr) PercentilePtr->ResetPercentileTableArray();
      else LogFprintfInfo("Statistic modules Tsc or Percentile are not instantiated\n");
      break;
#endif // __CONTROLABLE__
    case '0':
      if (LoggerPtr) {
        CommonDataPtr->LoggingLevel = LOG_LEVEL_ERROR;
        LoggerPtr->SetLogLevel(LOG_LEVEL_ERROR);
      } else LogFprintfInfo("Logging module is not instantiated\n");
      break;
    case '1':
      if (LoggerPtr) {
        CommonDataPtr->LoggingLevel = LOG_LEVEL_WARN;
        LoggerPtr->SetLogLevel(LOG_LEVEL_WARN);
      } else LogFprintfInfo("Logging module is not instantiated\n");
      break;
    case '2':
      if (LoggerPtr) {
        CommonDataPtr->LoggingLevel = LOG_LEVEL_INFO;
        LoggerPtr->SetLogLevel(LOG_LEVEL_INFO);
      } else LogFprintfInfo("Logging module is not instantiated\n");
      break;
    case '3':
      if (LoggerPtr) {
        CommonDataPtr->LoggingLevel = LOG_LEVEL_DEBUG;
        LoggerPtr->SetLogLevel(LOG_LEVEL_DEBUG);
      } else LogFprintfInfo("Logging module is not instantiated\n");
      break;
    case '?':
    case 'h':
    case 'H':
      //PrintUsage();
      LogFprintfInfo("\nControl keys: (use a <key><enter> sequence)\n");
      LogFprintfInfo("v Verbose  %d\n", CommonDataPtr->Verbose);
      LogFprintfInfo("b _FLAG_SHOW_HEARTBEAT_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_HEARTBEAT_ ? 1:0);
      LogFprintfInfo("u _FLAG_SHOW_DEC_DUMP_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_DEC_DUMP_ ? 1:0);
      LogFprintfInfo("x _FLAG_SHOW_HEX_DUMP_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_HEX_DUMP_ ? 1:0);
      LogFprintfInfo("V _FLAG_VERBOSE_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_VERBOSE_ ? 1:0);
      LogFprintfInfo("p _FLAG_SHOW_PARSER_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PARSER_ ? 1:0);
      LogFprintfInfo("P _FLAG_DONT_FILTER_SOUCE_ID_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_DONT_FILTER_SOUCE_ID_ ? 1:0);
      LogFprintfInfo("d _FLAG_SHOW_PUBLISHER_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_PUBLISHER_ ? 1:0);
      LogFprintfInfo("D _FLAG_SHOW_DATA_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_DATA_ ? 1:0);
#ifdef __CONTROLABLE__
      LogFprintfInfo("t _FLAG_SHOW_TIMING_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_SHOW_TIMING_ ? 1:0);
      LogFprintfInfo("T _FLAG_GET_TIMING_ %d\n", CommonDataPtr->ConfigurationFlags & _FLAG_GET_TIMING_ ? 1:0);
#endif // __CONTROLABLE__
      LogFprintfInfo("-/+ ConfigThreshold %d\n", CommonDataPtr->ConfigThreshold);
      LogFprintfInfo("s/S ConfigSleep %d\n", CommonDataPtr->ConfigSleep);
      LogFprintfInfo("m PrintSharedMemoryBlockPositions\n");
#ifdef __CONTROLABLE__
      LogFprintfInfo("%% PrintPercentileTableValues\n");
      LogFprintfInfo("# ResetPercentileTableArray\n");
#endif // __CONTROLABLE__
      LogFprintfInfo("0/1/2/3 SetLogLevel %d to  LOG_LEVEL_ERROR (%d) / LOG_LEVEL_WARN (%d) / LOG_LEVEL_INFO (%d) / LOG_LEVEL_DEBUG (%d)\n", CommonDataPtr->LoggingLevel, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG);
      break;
    default:
      break;
    }
  }
  return;
}

//#endif // __CONTROLABLE__


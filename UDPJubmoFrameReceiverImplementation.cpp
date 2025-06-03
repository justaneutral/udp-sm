
#include "UDPJubmoFrameReceiverImplementation.h"

UDPJubmoFrameReceiverImplementation::UDPJubmoFrameReceiverImplementation(int argc, char *argv[]) {
  ReturnValue = Initialize(argc, argv);
}


UDPJubmoFrameReceiverImplementation::~UDPJubmoFrameReceiverImplementation() {
 ReturnValue = Uninitialize();
}


int UDPJubmoFrameReceiverImplementation::GetReturnValue(void) {
  return ReturnValue;
}


int UDPJubmoFrameReceiverImplementation::Initialize(int argc, char *argv[]) {
  //Uninitialize();
  StreamState = __WAIT_FOR_SESSION__;
  ReturnValue = 0;
  std::cout << "Started = " << std::endl;
  LogFprintfInfo("\nArguments to main:\n");
  for (int i=0; i<argc; i++) {
    LogFprintfInfo("argv[%d]=%s\n", i, argv[i]);
  }
  /////1/////
  ArgumentsPtr = new InputArguments(argc, argv);
  /////2////
  LoggerPtr = (Logger *) NULL;
#ifndef _NULL_LOGGER_
  LoggerPtr = new Logger;
  if (LoggerPtr) {
    LoggerPtr->OpenLog((const char*) ArgumentsPtr->LoggingFileName, (LogLevelEnum) ArgumentsPtr->LoggingLevel, ArgumentsPtr->Verbose);
  } else {
    LogFprintfInfo("WARNUNG: Logger no initialized.\n");
  }
#endif //_NULL_LOGGER_
  /////3////
  ProcessorPtr = new ProcessorState(LoggerPtr);
  ReturnValue |= ProcessorPtr->StickThisThreadToProcessorCore(ArgumentsPtr->ProcessorCoreId);
  /////4////
  tsc = (Tsc *) NULL;
#ifndef _NULL_TSC_PERCENTILE_
  tsc = new Tsc;
  if (tsc) {
    tsc->MeasureTscOverhead();
  } else {
    LogFprintfInfo("WARNING: Time stamping module not initialized\n");
  }
#endif //_NULL_TSC_PERCENTILE_
  /////5/////
  percentile = (Percentile *) NULL;
#ifndef _NULL_TSC_PERCENTILE_
  percentile = new Percentile(LoggerPtr);
  if (!percentile) {
    LogFprintfInfo("WARNING: Statistics module not initialized\n");
  }
#endif //_NULL_TSC_PERCENTILE_
  /////6////
  RingBufferPtr0 = new RingFIFOBuffer(__RING_BUFFER_SIZE__, LoggerPtr);
  ReturnValue |= RingBufferPtr0->RetVal;
  RingBufferPtr1 = new RingFIFOBuffer(__RING_BUFFER_SIZE__, LoggerPtr);
  ReturnValue |= RingBufferPtr1->RetVal;
  /////7////
  CommonDataPtr = new CommonData(ArgumentsPtr->Verbose, ArgumentsPtr->SharedMemoryName, ArgumentsPtr->LoggingFileName, ArgumentsPtr->LoggingLevel);
  RunFlagSignal = &CommonDataPtr->ApplicationRunFlag;
  signal(SIGINT, signal_handler);
  /////8/////
  NetworkReceiverInterfacePtr = new NetworkReceiverImplementationEFVI;
  ReturnValue |= NetworkReceiverInterfacePtr->Initialize(CommonDataPtr, LoggerPtr, tsc, RingBufferPtr0, ArgumentsPtr->NetworkInterfaceName, ArgumentsPtr->IPv4MulticastUdpPort, ArgumentsPtr->IPv4MulticastAddress, ArgumentsPtr->ConfigDumpReceivedPacketsInHexadecimalFormat, ArgumentsPtr->ConfigDumpReceivedPacketsInAsciiFormat);
  /////9////
  ParserInterfacePtr = new ParserImplementation(CommonDataPtr, LoggerPtr, RingBufferPtr0, RingBufferPtr1);
  /////10////
  InserterInterfacePtr = new InserterImplementation; //&InserterInterfaceInst;
  ReturnValue |= InserterInterfacePtr->Initialize(CommonDataPtr, LoggerPtr, percentile, tsc, RingBufferPtr1);
  /////11////
  KeyboardPtr = new KeyboardReadInput(CommonDataPtr, InserterInterfacePtr, LoggerPtr, percentile, RingBufferPtr0, RingBufferPtr1);

  if(ReturnValue != 0) {
    CommonDataPtr->ApplicationRunFlag = 0;
    LogError("ef_vi or inserter initializing error\n");
  } else {
    //update WriterFlags to connected.
    InserterInterfacePtr->SetWriterFlags((__WriterFlagsShutdown__ | __WriterFlagsConnected__) & (~__WriterFlagsInSession__));
    LogFprintfInfo("Waiting for session_id\n");
  }
  return ReturnValue;
}

int UDPJubmoFrameReceiverImplementation::Iterate(void) {
  switch (StreamState) {
  case __WAIT_FOR_SESSION__:
    if(ReturnValue == 0) {
      if (CommonDataPtr->ApplicationRunFlag && !(CommonDataPtr->CommonFlagConnected && CommonDataPtr->CommonFlagSessionIdReceived)) {
        KeyboardPtr->ReadKeyboardInput();
        InserterInterfacePtr->WriteTimestampToSharedMemory();
        NetworkReceiverInterfacePtr->Iterate();
        if (CommonDataPtr->ApplicationRunFlag) {//0
          ParserInterfacePtr->Iterate();
        }//0
      } else {
        StreamState = __TRANSIT_TO_SESSION__;
      }
    }
    break;
  case __TRANSIT_TO_SESSION__:
    if (CommonDataPtr->ApplicationRunFlag && CommonDataPtr->CommonFlagConnected) {//-1
      LogFprintfInfo("received session_id\n");
      InserterInterfacePtr->PrintSessionIdFromSharedMemory();
      StreamState = __IN_SESSION__;
    }//-1
    break;
  case __IN_SESSION__:
    if(ReturnValue == 0) {
      if (CommonDataPtr->ApplicationRunFlag && CommonDataPtr->CommonFlagConnected) {
        InserterInterfacePtr->Iterate();
        NetworkReceiverInterfacePtr->Iterate();
        ParserInterfacePtr->Iterate();
        KeyboardPtr->ReadKeyboardInput();
      }
    }
    break;
  }
  return CommonDataPtr->ApplicationRunFlag;
}

int UDPJubmoFrameReceiverImplementation::Uninitialize(void) {

  ReturnValue = 0;

  if (InserterInterfacePtr) {
    ReturnValue = InserterInterfacePtr->Uninitialize();
    delete InserterInterfacePtr;
    InserterInterfacePtr = (InserterInterface *) NULL;
  }

  if (ParserInterfacePtr) {
    delete ParserInterfacePtr;
    ParserInterfacePtr = (ParserInterface *) NULL;
  }

  if (NetworkReceiverInterfacePtr) {
    ReturnValue |=  NetworkReceiverInterfacePtr->Uninitialize();
    delete NetworkReceiverInterfacePtr;
    NetworkReceiverInterfacePtr = (NetworkReceiverInterface *) NULL;
  }

  if (RingBufferPtr0) {
    RingBufferPtr0->PrintStatus("receiver to parser");
    RingBufferPtr0->Uninitialize();
    delete RingBufferPtr0;
    RingBufferPtr0 = (RingFIFOBuffer *) NULL;
  }

  if (RingBufferPtr1) {
    RingBufferPtr1->PrintStatus("parser tp inserter");
    RingBufferPtr1->Uninitialize();
    delete RingBufferPtr1;
    RingBufferPtr1 = (RingFIFOBuffer *) NULL;
  }

  if (CommonDataPtr) {
    delete CommonDataPtr;
    CommonDataPtr = (CommonData *) NULL;
  }

  if (KeyboardPtr) {
    delete KeyboardPtr;
    KeyboardPtr = (KeyboardReadInput *) NULL;
  }

  if (percentile) {
    delete percentile;
    percentile = (Percentile *) NULL;
  }

  if (tsc) {
    delete tsc;
    tsc = (Tsc *) NULL;
  }

  if (ProcessorPtr) {
    delete ProcessorPtr;
    ProcessorPtr = (ProcessorState *) NULL;
  }

  if (ArgumentsPtr) {
    delete ArgumentsPtr;
    ArgumentsPtr = (InputArguments *) NULL;
  }

  LOGV("\nshm ret val = %d\n", ReturnValue);
  if (LoggerPtr) {
    LoggerPtr->CloseLog();
    delete LoggerPtr;
    LoggerPtr = (Logger *) NULL;
  }

 // ReturnValue = 0;
  LogFprintfInfo("\nshm uninitialize ret val = %d\n", ReturnValue);

  return ReturnValue;
}

// _eof_


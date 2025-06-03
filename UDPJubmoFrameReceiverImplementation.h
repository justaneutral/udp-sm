
#ifndef __NATIVE_STREAM_READER_IMPLEMENTATION__
#define __NATIVE_STREAM_READER_IMPLEMENTATION__

#include "UDPJubmoFrameReceiverInterface.h"

#include <iostream>
#include <stdio.h>
#include "Logger.h"
#include "Percentile.h"
#include "InputArguments.h"
#include "KeyboardReadInput.h"
#include "NetworkReceiverInterface.h"
#include "ParserInterface.h"
#include "ParserImplementation.h"
#include "Tsc.h"
#include "InserterInterface.h"
#include "InserterImplementation.h"
#include "ProcessorState.h"


////////////////

//#define _NULL_LOGGER_
//#define _NULL_TSC_PERCENTILE_

/////////////////////////////  MAIN SECTION  /////////////////////////////////
class UDPJubmoFrameReceiverImplementation : public UDPJubmoFrameReceiverInterface {
public:
UDPJubmoFrameReceiverImplementation(int argc, char *argv[]);
~UDPJubmoFrameReceiverImplementation();
int Initialize(int argc, char *argv[]);
int Iterate(void);
int Uninitialize(void);
int GetReturnValue(void);
private:
int ReturnValue;
enum StreamStatusEnum_t { __WAIT_FOR_SESSION__ = 0, __TRANSIT_TO_SESSION__, __IN_SESSION__ };
enum StreamStatusEnum_t StreamState;
InputArguments *ArgumentsPtr;
Logger *LoggerPtr;
ProcessorState *ProcessorPtr;
Tsc *tsc;
Percentile *percentile;
RingFIFOBuffer *RingBufferPtr0;
RingFIFOBuffer *RingBufferPtr1;
CommonData *CommonDataPtr;
NetworkReceiverInterface* NetworkReceiverInterfacePtr;
ParserInterface *ParserInterfacePtr;
InserterInterface *InserterInterfacePtr;
KeyboardReadInput *KeyboardPtr;
};

#endif // __NATIVE_STREAM_READER_IMPLEMENTATION__
// _eof_



#ifndef __NATIVE_STREAM_READER_INTERFACE__
#define __NATIVE_STREAM_READER_INTERFACE__

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


class UDPJubmoFrameReceiverInterface {
public:
 virtual ~UDPJubmoFrameReceiverInterface() {};
 virtual int Initialize(int argc, char *argv[]) =0;
 virtual int Iterate(void) = 0;
 virtual int Uninitialize(void) = 0;
 virtual int GetReturnValue(void) = 0;
};

#endif // __NATIVE_STREAM_READER_INTERFACE__
// _eof_


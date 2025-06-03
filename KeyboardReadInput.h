#ifndef __KEYBOARD_READ_INPUT__
#define __KEYBOARD_READ_INPUT__

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "defines.h"
#include "utils.h"
#include "flags.h"
#include "CommonData.h"
#include "Percentile.h"
#include "Logger.h"
#include "InserterInterface.h"

#ifdef __CONTROLABLE__
#include "Percentile.h"
#endif // __CONTROLABLE__

#include "Tsc.h"



extern int *RunFlagSignal;
void signal_handler(int ProgramInterruptSignalNumber);
class  KeyboardReadInput {
public:
  KeyboardReadInput(CommonData *CommonDataPtr, InserterInterface *InserterInterfacePtr, Logger *LoggerPtr, Percentile *PercentilePtr, RingFIFOBuffer *RingBufferPtr0, RingFIFOBuffer *RingBufferPtr1);
  ~KeyboardReadInput();
  int KeyboardHit(void);
  void ReadKeyboardInput(void);

  CommonData *CommonDataPtr;
  InserterInterface *InserterInterfacePtr;
  Logger *LoggerPtr;
  Percentile *PercentilePtr;
  RingFIFOBuffer *RingBufferPtr0;
  RingFIFOBuffer *RingBufferPtr1;
};

#endif // __KEYBOARD_READ_INPUT__


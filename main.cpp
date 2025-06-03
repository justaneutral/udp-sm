
#include "UDPJubmoFrameReceiverImplementation.h"

int main(int argc, char *argv[]) {
  int RetVal = -1;
  UDPJubmoFrameReceiverInterface *StreamReaderPtr = new UDPJubmoFrameReceiverImplementation(argc, argv);
  if (StreamReaderPtr) {
    RetVal = StreamReaderPtr->GetReturnValue();
    if (RetVal == 0) {
      while (StreamReaderPtr->Iterate());
    }
    RetVal = StreamReaderPtr->Uninitialize();
    delete StreamReaderPtr;
  }
  return RetVal;
}

// _eof_


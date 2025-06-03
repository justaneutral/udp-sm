#ifndef __INPUT_ARGUMENTS__
#define __INPUT_ARGUMENTS__

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "defines.h"
#include "defaults.h"
#include "utils.h"
#include "mjson.h"
#include "NetworkReceiverImplementationEFVI.h"


class InputArguments {
public:
  InputArguments(int argc, char *argv[]);
  ~InputArguments();
  int ReadParameters(int argc, char *argv[]);
  void PrintUsage(void);
  char *ReadJsonStringFromFile(char *JsonParametersFileName);
  int ReturnValue;
  int status;

  // arguments for app_ef_vi_state_ptr
  int ProcessorCoreId; // for app_ef_vi_state_ptr->ProcessorCoreId;
  char NetworkInterfaceName[IFNAMSIZ]; //for app_ef_vi_state_ptr->cfg.NetworkInterfaceName
  int IPv4MulticastUdpPort; // for app_ef_vi_state_ptr->cfg.port
  char IPv4MulticastAddress[INET_ADDRSTRLEN]; // for  app_ef_vi_state_ptr->cfg.IPv4MulticastAddress
  bool ConfigDumpReceivedPacketsInAsciiFormat, ConfigDumpReceivedPacketsInHexadecimalFormat; // for app_ef_vi_state_ptr->cfg.ConfigDumpReceivedPacketsInHexadecimalFormat, app_ef_vi_state_ptr->cfg.ConfigDumpReceivedPacketsInAsciiFormat

  // arguments for CommonDataPtr
  bool Verbose; // for CommonDataPtr->Verbose
  char SharedMemoryName[__SHM_NAME_LENGTH__]; // for ommon_data_ptr->SharedMemoryName
  char LoggingFileName[LOGNAMSIZ]; // for CommonDataPtr->LoggingFileName
  int LoggingLevel; // for  CommonDataPtr->LoggingLevel
};

#endif // __INPUT_ARGUMENTS__



#include "InputArguments.h"

///////////////// parameters ////////////////////////////
InputArguments::InputArguments(int argc, char *argv[]) {
  ReturnValue = ReadParameters(argc, argv);
}

InputArguments::~InputArguments(void) {}

int InputArguments::ReadParameters(int argc, char *argv[]) {
  //reset to uninitialized values
  ProcessorCoreId = 0;
  SharedMemoryName[0] = 0;
  NetworkInterfaceName[0] = 0;
  Verbose = 0;
  IPv4MulticastUdpPort = 0;
  IPv4MulticastAddress[0] = 0;
  ConfigDumpReceivedPacketsInAsciiFormat = 0;
  ConfigDumpReceivedPacketsInHexadecimalFormat = 0;
  LoggingFileName[0] = 0;
  LoggingLevel = 0;
  if (argc<2) { //file name is presen
    LogFprintfInfo("Not enough input parameters: need json file name.\n");
    PrintUsage();
    return -1;
  }
  char *JsonInputString = ReadJsonStringFromFile(argv[1]);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  struct json_attr_t JsonAttributes[] = {
    {(char *) "core_id", t_integer,},
    {(char *) "shm_name", t_string,},
    {(char *) "iface", t_string,},
    {(char *) "verbose",   t_boolean,},
    {(char *) "port",    t_integer,},
    {(char *) "ip_multicast", t_string,},
    {(char *) "dump_pkt",   t_boolean,},
    {(char *) "dump_hex",   t_boolean,},
    {(char *) "log_name", t_string,},
    {(char *) "log_level", t_integer,},
    {NULL},
  };


#pragma GCC diagnostic pop
  JsonAttributes[0].addr.integer = &ProcessorCoreId;
  JsonAttributes[1].addr.string = SharedMemoryName; JsonAttributes[1].len = __SHM_NAME_LENGTH__;
  JsonAttributes[2].addr.string = NetworkInterfaceName; JsonAttributes[2].len = IFNAMSIZ;
  JsonAttributes[3].addr.boolean = &Verbose;
  JsonAttributes[4].addr.integer = &IPv4MulticastUdpPort;
  JsonAttributes[5].addr.string = IPv4MulticastAddress; JsonAttributes[5].len = INET_ADDRSTRLEN;
  JsonAttributes[6].addr.boolean = &ConfigDumpReceivedPacketsInAsciiFormat;
  JsonAttributes[7].addr.boolean = &ConfigDumpReceivedPacketsInHexadecimalFormat;
  JsonAttributes[8].addr.string = LoggingFileName; JsonAttributes[8].len = LOGNAMSIZ;
  JsonAttributes[9].addr.integer = &LoggingLevel;
  int status = 0;
  status = json_read_object(JsonInputString, JsonAttributes, NULL);
  ConfigDumpReceivedPacketsInAsciiFormat = ConfigDumpReceivedPacketsInHexadecimalFormat ? 0 : ConfigDumpReceivedPacketsInAsciiFormat;
  LOGV("input parameters: status = %d, ProcessorCoreId = %d, SharedMemoryName = %s, NetworkInterfaceName = %s, IPv4MulticastUdpPort = %d, IPv4MulticastAddress = %s, ConfigDumpReceivedPacketsInAsciiFormat = %d, ConfigDumpReceivedPacketsInHexadecimalFormat = %d, LoggingFileName = %s, LoggingLevel = %d, Verbose = %d\n", status, ProcessorCoreId, SharedMemoryName, NetworkInterfaceName, IPv4MulticastUdpPort, IPv4MulticastAddress, ConfigDumpReceivedPacketsInAsciiFormat, ConfigDumpReceivedPacketsInHexadecimalFormat, LoggingFileName[0] ? LoggingFileName : "Not defined" , LoggingLevel,  Verbose);
  if (status != 0) {
    LogFprintfInfo("Errors in input parameters: need correct json input file. Error code: %s\n", json_error_string(status));
    PrintUsage();
  } else {
    LogFprintfInfo("status = %d, ProcessorCoreId = %d, SharedMemoryName = %s, NetworkInterfaceName = %s, IPv4MulticastUdpPort = %d, IPv4MulticastAddress = %s, ConfigDumpReceivedPacketsInAsciiFormat = %d, ConfigDumpReceivedPacketsInHexadecimalFormat = %d, LoggingFileName = %s, LoggingLevel = %d, Verbose = %d\n", status, ProcessorCoreId, SharedMemoryName, NetworkInterfaceName, IPv4MulticastUdpPort, IPv4MulticastAddress, ConfigDumpReceivedPacketsInAsciiFormat, ConfigDumpReceivedPacketsInHexadecimalFormat, LoggingFileName[0] ? LoggingFileName : "Not defined", LoggingLevel, Verbose);
  }
  return status;
}

char *InputArguments::ReadJsonStringFromFile(char *JsonParametersFileName) {
  static char buffer[4096];
  size_t length;
  FILE * f = fopen (JsonParametersFileName, "rb");
  memset(buffer, 0, 4096);
  if (f) {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    if (length < 4096) {
      fseek (f, 0, SEEK_SET);
      if(length != fread (buffer, 1, length, f)) {
        memset(buffer, 0, length);
      }
    }
    fclose (f);
  }
  return buffer;
}

void InputArguments::PrintUsage(void) {
  LogFprintfInfo ("PrintUsage:\n");
  LogFprintfInfo ("efjumborx <json file name>\n");
  LogFprintfInfo ("Example json string: \"{\"ProcessorCoreId\": 12, \"SharedMemoryName\": \"/dev/hugepages/ef_vi_2mb0.mmap\", \"NetworkInterfaceName\": \"mint1\", \"Verbose\": false, \"IPv4MulticastUdpPort\": 32002, \"IPv4MulticastAddress\": \"239.2.3.12\", \"ConfigDumpReceivedPacketsInAsciiFormat\": false, \"ConfigDumpReceivedPacketsInHexadecimalFormat\": false, \"LoggingFileName\": \"log.txt\", \"LoggingLevel\": 3 }\"\n");
  LogFprintfInfo ("json parameters:\n");
  LogFprintfInfo ("\"ProcessorCoreId\": 1-16 (depends on max number of processor cores)");
  LogFprintfInfo ("\"SharedMemoryName\": path to a shared mamory file used for message inserting\n");
  LogFprintfInfo ("\"NetworkInterfaceName\": network interface name\n");
  LogFprintfInfo ("\"Verbose\": Enables additional logging output if set\n");
  LogFprintfInfo ("\"ConfigDumpReceivedPacketsInAsciiFormat\": Writes the packet payload to stdout as text.Cannot be set in conjunction with ConfigDumpReceivedPacketsInHexadecimalFormat\n");
  LogFprintfInfo ("\"ConfigDumpReceivedPacketsInHexadecimalFormat\": dump packet contents as hex. Writes the packet payload to stdout. Cannot be set in conjunction with ConfigDumpReceivedPacketsInAsciiFormat\n");
  LogFprintfInfo ("\"IPv4MulticastAddress\": Set the destination IP. Default is %s\n",DEST_IPADDR);
  LogFprintfInfo ("\"IPv4MulticastUdpPort\": Set the destination port. Default is %d\n",DEST_PORT);
  LogFprintfInfo ("\"LoggingFileName\": path to a log file used for error / warn / info / debug  message logging\n");
  LogFprintfInfo ("\"LoggingLevel\": required logging level:0 ( error) / 1 (warn) / 2 (info) / 3 (debug)\n");
  LogFprintfInfo ("\n");
}

// _eof



#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <cstring>

// Define log levels
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevelEnum;

// Data structure for log
class Logger {
public:
  Logger();
  ~Logger();
  // Function declarations
  void OpenLog(const char* FileNamep, LogLevelEnum Level, int Verbose);
  void SetLogLevel(LogLevelEnum Level);
  void SetLogVerbose(int Verbose);
  void CloseLog(void);

  FILE* file;
  LogLevelEnum LoggingLevel;
  int Verbose;
};

#define STD_PRINT(...) \
  do { \
    if(LoggerPtr) do { \
      if (LoggerPtr->LoggingLevel >= LOG_LEVEL_INFO) do { \
        fprintf(stdout, __VA_ARGS__); \
        if (LoggerPtr->Verbose) do { \
          fprintf(stdout, "File = %s, Line = %d\n", __FILE__, __LINE__); \
        } while (0);\
      } while (0); \
      else do { \
        fprintf(stderr, __VA_ARGS__); \
        if (LoggerPtr->Verbose) do { \
          fprintf(stderr, "File = %s, Line = %d\n", __FILE__, __LINE__); \
        } while (0); \
      } while (0); \
    } while (0); \
  } while (0);


#define LOG_PRINT(x, ...) \
  do { \
    STD_PRINT(__VA_ARGS__); \
    if (LoggerPtr) do { \
      if (LoggerPtr->file) do { \
        if(LoggerPtr->LoggingLevel >=  (x)) do { \
          fprintf(LoggerPtr->file, __VA_ARGS__); \
          if (LoggerPtr->Verbose) do { \
            fprintf(LoggerPtr->file, "File = %s, Line = %d\n", __FILE__, __LINE__); \
          } while (0); \
        } while (0); \
      } while( 0 ); \
    } while( 0 ); \
  } while( 0 );


#define LogError(...) do{ LOG_PRINT(LOG_LEVEL_ERROR, __VA_ARGS__); } while (0);
#define LogWarn(...) do{ LOG_PRINT(LOG_LEVEL_WARN, __VA_ARGS__); } while (0);
#define LogInfo(...) do{ LOG_PRINT(LOG_LEVEL_INFO, __VA_ARGS__); } while (0);
#define LogDebug(...) do{ LOG_PRINT(LOG_LEVEL_DEBUG, __VA_ARGS__); } while (0);
#define Print8BitValueWithAddress(q)  do { if(q) { LogDebug("\t%s\t=\t(%016lX)\t[%02X]\n",  #q, (uint64_t)((uint8_t  *)q), *(uint8_t *)q);} else { LogDebug("\t%s\tNOT INITIALIZED\n", #q);}} while(0);
#define Print16BitValueWithAddress(q) do { if(q) { LogDebug("\t%s\t=\t(%016lX)\t[%04X]\n",  #q, (uint64_t)((uint16_t *)q), ntohs(*(uint16_t *)q));} else { LogDebug("\t%s\tNOT INITIALIZED\n", #q);}} while(0);
#define Print32BitValueWithAddress(q) do { if(q) { LogDebug("\t%s\t=\t(%016lX)\t[%08lX]\n", #q, (uint64_t)((uint32_t *)q), ntohl(*(uint32_t *)q));} else { LogDebug("\t%s\tNOT INITIALIZED\n", #q);}} while(0);
#define Print64BitValueWithAddress(q) do { if(q) { LogDebug("\t%s\t=\t(%016lX)\t[%016lX]\n", #q, (uint64_t)((uint64_t *)q), ntoh64(*(uint64_t *)q));} else { LogDebug("\t%s\tNOT INITIALIZED\n", #q);}} while(0);
#define Print8BitValue(q)  LogDebug("\t%s\t=\t%u\n", #q,(uint8_t)q);
#define Print16BitValue(q) LogDebug("\t%s\t=\t%u\n", #q,(uint16_t)q);
#define Print32BitValue(q) LogDebug("\t%s\t=\t%u\n", #q,(uint32_t)q);
#define Print64BitValue(q) LogDebug("\t%s\t=\t%lu\n",#q,(uint64_t)q);
#endif //__LOGGER_H__



#include "Logger.h"

Logger::Logger()
{
  file = NULL;
}

Logger::~Logger()
{
  CloseLog();
}

// Function to open log file
void Logger::OpenLog(const char* FileName, LogLevelEnum LoggingLevel, int Verbose) {
  if (FileName && strlen(FileName)) file = fopen(FileName, "a");
  else file = (FILE *) NULL;
  SetLogLevel(LoggingLevel);
  SetLogVerbose(Verbose);
}

// Function to close log file
void Logger::CloseLog(void) {
  if (file) {
    fclose(file);
    file = NULL;
  }
}

void Logger::SetLogLevel(LogLevelEnum LoggingLevel) {
  if (LoggingLevel < 0) this->LoggingLevel = (LogLevelEnum) 0;
  else if (LoggingLevel > 3) this->LoggingLevel = (LogLevelEnum) 3;
  else this->LoggingLevel = LoggingLevel;
}

void Logger::SetLogVerbose(int Verbose) {
    this->Verbose = Verbose ? 1 : 0;
}

// _eof


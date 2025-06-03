
#ifndef __PERCENTILE__
#define __PERCENTILE__

#include "utils.h"
#include "Logger.h"

#define __NUM_PERCENTILES__ (10)
#define __NUM_PERCENTILE_MEASUREMENTS__ (1000)

// Structure to store MeasurementArray
class Percentile {
public:
  Percentile(Logger *LoggerPtr);
  ~Percentile();
  // Initialize percentile_t
  void InitializePercentileTable(int NumberOfPercentiles, int MaximumNumberOfMeasurementses, Logger *LoggerPtr);
  void AddNanosecondTimeStampMeasurement(uint64_t MeasuredValue);
  void PrintPercentileTableValues(void);
  void ResetPercentileTableArray(void);
private:
  uint64_t MeasurementArray[__NUM_PERCENTILE_MEASUREMENTS__];
  int CurrentNumberOfMeasurementses;
  int MaximumNumberOfMeasurementses;
  int NumberOfPercentiles;
public:
  Logger *LoggerPtr;
};


#endif //__PERCENTILE__
// __eof__


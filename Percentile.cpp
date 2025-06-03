
#include "Percentile.h"


Percentile::Percentile(Logger *LoggerPtr) {
  const int NumberOfPercentiles = __NUM_PERCENTILES__;
  const int MaximumNumberOfMeasurementses = __NUM_PERCENTILE_MEASUREMENTS__;
  InitializePercentileTable((int) NumberOfPercentiles, (int) MaximumNumberOfMeasurementses, LoggerPtr);
  LogFprintfInfo("Percentile table initialized:\n");
}


Percentile::~Percentile() {
  ResetPercentileTableArray();
}

// Initialize percentile_t
void Percentile::InitializePercentileTable(int NumberOfPercentiles, int MaximumNumberOfMeasurementses, Logger *LoggerPtr) {
  this->LoggerPtr = LoggerPtr;
  if((NumberOfPercentiles > 0) && (MaximumNumberOfMeasurementses > 0)) {
    ResetPercentileTableArray();
    this->NumberOfPercentiles = NumberOfPercentiles;
    this->MaximumNumberOfMeasurementses = MaximumNumberOfMeasurementses;
    for (int i = 0; i < MaximumNumberOfMeasurementses; i++) {
      AddNanosecondTimeStampMeasurement((uint64_t) i);
    }
    PrintPercentileTableValues();
    ResetPercentileTableArray();
  }
}

// Add a measurement
void Percentile::AddNanosecondTimeStampMeasurement(uint64_t MeasuredValue) {
  if (CurrentNumberOfMeasurementses < MaximumNumberOfMeasurementses) {
    MeasurementArray[CurrentNumberOfMeasurementses++] = MeasuredValue;
  }
}

// Compare function for sorting
static int CompareValuesForSorting(const void* CompareValuePtr1, const void* CompareValuePtr2) {
  return (*(uint64_t*)CompareValuePtr1 - *(uint64_t*)CompareValuePtr2);
}

// Print percentile table
void Percentile::PrintPercentileTableValues(void) {
  if (CurrentNumberOfMeasurementses >=  MaximumNumberOfMeasurementses) {
#if(1)
    qsort(MeasurementArray, CurrentNumberOfMeasurementses, sizeof(uint64_t), CompareValuesForSorting);
#else
    for (int i = 0; i < MaximumNumberOfMeasurementses; i++) {
      for (int j = 0; j < (MaximumNumberOfMeasurementses - 1); j++) {
        uint64_t TempSwapVal;
        if(MeasurementArray[j] > MeasurementArray[j+1]) {
          TempSwapVal = MeasurementArray[j];
          MeasurementArray[j] = MeasurementArray[j+1];
          MeasurementArray[j+1] = TempSwapVal;
        }
      }
    }
#endif
    LogFprintfInfo("Percentile Table:\n");
    LogFprintfInfo("------------------\n");

    for (int i = 0; i < NumberOfPercentiles; ++i) {
      double PercentileValueAtIndex = (i + 1) * 100.0 / NumberOfPercentiles;
      int PercentileIndex = (int)((PercentileValueAtIndex / 100.0) * CurrentNumberOfMeasurementses)-1;
      uint64_t ValueAtIndex = MeasurementArray[PercentileIndex];
      LogFprintfInfo("P%03.1f: %lu\n", PercentileValueAtIndex, ValueAtIndex);
    }
  }
}

// Reset percentile table (keep allocated memory)
void Percentile::ResetPercentileTableArray(void) {
  CurrentNumberOfMeasurementses = 0;
}

// _eof_


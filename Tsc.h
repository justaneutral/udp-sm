#include "defines.h"
#ifdef __CONTROLABLE__
#ifndef TSC_H
#define TSC_H

#include <stdint.h>
#define TSC_OVERHEAD_N 100000
#define TSC_FREQ 2.9 //GHz

class Tsc {
public:
  Tsc();
  ~Tsc();
  // BenchStart returns a times tamp for use to measure the start of a benchmark run.
  uint64_t BenchStart(void);
  // BenchEnd returns a time stamp for use to measure the end of a benchmark run.
  uint64_t BenchEnd(void);
  uint64_t MeasureTscOverhead(void);
  uint64_t GetTimestamp(void);
  uint64_t *GetTimestamp_ptr(void);
  void SetTimestamp(uint64_t TimestampValue);
  uint64_t GetLatency(void);

private:
  uint64_t TscOverheadCycles;
  uint64_t TscStartCycles;
  uint64_t TscEndCycles;
  uint64_t TscTimeStamp;
  uint64_t TscLatencyValue;
};
#endif /* TSC_H */
#endif //__CONTROLABLE__

// _eof_


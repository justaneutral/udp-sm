
#include "Tsc.h"
#ifdef __CONTROLABLE__

Tsc::Tsc() {
  TscOverheadCycles = MeasureTscOverhead();
}

Tsc::~Tsc() {}

// BenchStart returns a time stamp for use to measure the start of a benchmark
// run.
uint64_t Tsc::BenchStart(void) {
  unsigned  cycles_low, cycles_high;
  asm volatile( "CPUID\n\t" // serialize
                "RDTSC\n\t" // read clock
                "MOV %%edx, %0\n\t"
                "MOV %%eax, %1\n\t"
                : "=r" (cycles_high), "=r" (cycles_low)
                :: "%rax", "%rbx", "%rcx", "%rdx" );
  TscStartCycles =  ((uint64_t) cycles_high << 32) | cycles_low;
  TscTimeStamp = (uint64_t)(TscStartCycles + TscOverheadCycles);
  return TscStartCycles;
}

// BenchEnd returns a time stamp for use to measure the end of a benchmark run.
uint64_t Tsc::BenchEnd(void) {
  unsigned  cycles_low, cycles_high;
  asm volatile( "RDTSCP\n\t" // read clock + serialize
                "MOV %%edx, %0\n\t"
                "MOV %%eax, %1\n\t"
                "CPUID\n\t" // serialze -- but outside clock region!
                : "=r" (cycles_high), "=r" (cycles_low)
                :: "%rax", "%rbx", "%rcx", "%rdx" );
  TscEndCycles =  ((uint64_t) cycles_high << 32) | cycles_low;
  return TscEndCycles;
}

// MeasureTscOverhead returns the overhead from benchmarking, it should be
// subtracted from timings to improve accuracy.

#ifndef __TSC_DONT_MEASURE_OVERHEAD__

uint64_t Tsc::MeasureTscOverhead(void) {
  uint64_t t0, t1, overhead = ~0;
  int i;

  for (i = 0; i < TSC_OVERHEAD_N; i++) {
    t0 = BenchStart();
    asm volatile("");
    t1 = BenchEnd();
    if (t1 - t0 < overhead) {
      overhead = t1 - t0;
    }
  }
  TscOverheadCycles = overhead;
  return overhead;
}

#else //__TSC_DONT_MEASURE_OVERHEAD__

uint64_t Tsc::MeasureTscOverhead(void) {
  TscOverheadCycles = 0;
  return 0;
}

#endif //__TSC_DONT_MEASURE_OVERHEAD__

uint64_t Tsc::GetTimestamp(void) {
  return TscTimeStamp;
}

uint64_t *Tsc::GetTimestamp_ptr(void) {
  return &TscTimeStamp;
}

void Tsc::SetTimestamp(uint64_t TimestampValue) {
  TscTimeStamp = TimestampValue;
}

uint64_t Tsc::GetLatency(void) {
  TscLatencyValue = TscEndCycles - TscTimeStamp;
  return TscLatencyValue;
}

#endif //__CONTROLABLE__
// _eof_


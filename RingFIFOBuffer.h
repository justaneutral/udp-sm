// RingFIFOBuffer.h
#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "utils.h"
#include "Logger.h"

#define __LOG2A(s) ((s &0xffffffff00000000) ? (32 +__LOG2B(s >>32)): (__LOG2B(s)))
#define __LOG2B(s) ((s &0xffff0000)         ? (16 +__LOG2C(s >>16)): (__LOG2C(s)))
#define __LOG2C(s) ((s &0xff00)             ? (8  +__LOG2D(s >>8)) : (__LOG2D(s)))
#define __LOG2D(s) ((s &0xf0)               ? (4  +__LOG2E(s >>4)) : (__LOG2E(s)))
#define __LOG2E(s) ((s &0xc)                ? (2  +__LOG2F(s >>2)) : (__LOG2F(s)))
#define __LOG2F(s) ((s &0x2)                ? (1)                  : (0))

#define LOG2_UINT64 __LOG2A
#define LOG2_UINT32 __LOG2B
#define LOG2_UINT16 __LOG2C
#define LOG2_UINT8  __LOG2D


typedef struct Snap_str_t {
  int buffer_size;
  int write_index;
  int commit_index;
  int read_index;
  int NumberOfStoredBytes;
  int FramesStored;
  int max_frames;
  int max_bytes;
  int min_capacity;
  uint64_t NumberOfProcessedFrames;
} __attribute__((packed)) Snap_t, *pSnap_t;

typedef union ulen_u_t {
  uint32_t l;
  uint8_t b[4];
} __attribute__((packed)) ulen_t;


typedef struct ringbuffer_str_t {
  int buffer_size;
  int write_index;
  int commit_index;
  int read_index;
  int NumberOfStoredBytes;
  int FramesStored;
  int max_frames;
  int max_bytes;
  int min_capacity;
  uint64_t NumberOfProcessedFrames;
  int SlipCounter;
  unsigned char *ptr;
} __attribute__((packed)) ringbuffer_t, *pringbuffer_t;


class RingFIFOBuffer {
public:
  RingFIFOBuffer();
  RingFIFOBuffer(size_t BufferSizeBytes, Logger * LoggerPtr);
  ~RingFIFOBuffer();
  int Initialize(size_t BufferSizeBytes, Logger * LoggerPtr);
  int Write(unsigned char *data, uint32_t length);
  int Read(unsigned char *data);
  int GetCapacity(void);
  int Uninitialize(void);
  void PrintStatus(const char *identification);
  void PrintSnapStatus(const char *identification);
  void Snap(void);
  void Discard(void);
  //////////////// testing //////////////////////
  uint64_t Next64AlignedTest(uint64_t x);
  Logger *LoggerPtr;
private:
  Snap_t ringbufsnap;
public:
  ringbuffer_t ringbuf;
  int RetVal;
};

#endif //__RINGBUFFER_H__
// EOF RingFIFOBuffer.h


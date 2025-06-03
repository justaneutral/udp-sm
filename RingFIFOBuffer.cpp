
#include "RingFIFOBuffer.h"

//#define __DEBUG_RINGBUFFER_CATCH_ZERO_INSERTS__

static inline uint64_t nextpow2(uint64_t i) {
#if defined(__GNUC__)
  return 1UL <<(1 +(63 -__builtin_clzl(i -1)));
#else
  i =i -1;
  i =LOG2_UINT64(i);
  return 1UL <<(1 +i);
#endif
}


static inline uint64_t next64aligned(uint64_t x) {
  const uint64_t mask = ~0x3f;
  uint64_t y = (x+63) & mask;
  return y;
}


RingFIFOBuffer::RingFIFOBuffer() {
  size_t BufferSizeBytes = (size_t) 0;
  Logger * LoggerPtr = (Logger *) NULL;
  RingFIFOBuffer(BufferSizeBytes, LoggerPtr);
}


RingFIFOBuffer::RingFIFOBuffer(size_t BufferSizeBytes, Logger * LoggerPtr) {
  RetVal = Initialize(BufferSizeBytes, LoggerPtr);
}

RingFIFOBuffer::~RingFIFOBuffer() {
  RetVal = Uninitialize();
}


int RingFIFOBuffer::Initialize(size_t BufferSizeBytes, Logger * LoggerPtr) {
  int ReturnVal = 0;
  this->LoggerPtr = LoggerPtr;
  pringbuffer_t p = &ringbuf;
  size_t ResultingCapacity = 0;
  if(p) {
    p->buffer_size = 0;
    p->write_index = 0;
    p->commit_index = 0;
    p->read_index = 0;
    p->NumberOfStoredBytes = 0;
    p->FramesStored = 0;
    p->max_bytes = 0;
    p->max_frames = 0;
    p->min_capacity = 0;
    p->NumberOfProcessedFrames = 0;
    p->SlipCounter = 0;
    p->ptr = NULL;
    if(BufferSizeBytes) {
      size_t p2size = nextpow2(BufferSizeBytes);
      p->ptr = (unsigned char *) new unsigned char[p2size];
      if(p->ptr) {
        memset(p->ptr, 0, p2size);
        p->buffer_size = (int)p2size;
        p->min_capacity =  p->buffer_size;
        ResultingCapacity = p->buffer_size;
      }
      if (BufferSizeBytes > ResultingCapacity) ReturnVal = -1 ;
    }
    LogDebug("Initialize:\n\tsize = %d, write_index = %d, commit_index = %d, read_index = %d\n", p->buffer_size, p->write_index, p->commit_index, p->read_index);
  }
  return ReturnVal;
}

int RingFIFOBuffer::Uninitialize(void) {
  int ReturnVal = -1;
  pringbuffer_t p = &ringbuf;
  if(p) {
    PrintStatus("***");
    LOGV("Uninitialize:\n\tmin_capacity = %d, max_frames = %d, max_bytes = %d\n", p->min_capacity, p->max_frames, p->max_bytes);
    LOGV("Processed %lu frames, currently stored in buffer:\tframes = %d, bytes = %d\n", p->NumberOfProcessedFrames, p->FramesStored, p->NumberOfStoredBytes);
    //fflush(stdout);
    if((p->buffer_size > 0) && (p->ptr)) {
      delete [] p->ptr;
      p->ptr = NULL;
     ReturnVal = 0;
    }
    p->buffer_size = 0;
    p->write_index = 0;
    p->commit_index = 0;
    p->read_index = 0;
    p->NumberOfStoredBytes = 0;
    p->FramesStored = 0;
    p->min_capacity = 0;
    p->max_frames = 0;
    p->max_bytes = 0;
    p->NumberOfProcessedFrames = 0;
  }
  return ReturnVal;
}

void RingFIFOBuffer::PrintStatus(const char *identification) {
  pringbuffer_t p = &ringbuf;
  if(p) {
    LogFprintfInfo("ringbuffer %s : size = %d\n\tmin_capacity = %d, max_frames = %d, max_bytes = %d, write_index = %d, commit_index = %d, read_index = %d\n", identification, p->buffer_size, p->min_capacity, p->max_frames, p->max_bytes, p->write_index, p->commit_index, p->read_index);
    LogFprintfInfo("Processed %lu frames, currently stored in buffer:\tframes = %d, bytes = %d, SlipCounter = %d\n", p->NumberOfProcessedFrames, p->FramesStored, p->NumberOfStoredBytes, p->SlipCounter);
  } else {
    LogDebug("ringbuffer %s is uninitialized\n", identification);
  }
}


void RingFIFOBuffer::PrintSnapStatus(const char *identification) {
  pringbuffer_t p = (pringbuffer_t)&ringbufsnap;
  if(p) {
    LogFprintfInfo("ringbuffer %s : size = %d\n\tmin_capacity = %d, max_frames = %d, max_bytes = %d, write_index = %d, commit_index = %d, read_index = %d\n", identification, p->buffer_size, p->min_capacity, p->max_frames, p->max_bytes, p->write_index, p->commit_index, p->read_index);
    LogFprintfInfo("Processed %lu frames, currently stored in buffer:\tframes = %d, bytes = %d, SlipCounter = %d\n", p->NumberOfProcessedFrames, p->FramesStored, p->NumberOfStoredBytes, p->SlipCounter);
    //fflush(stdout);
  } else {
    LogDebug("ringbuffer %s is uninitialized\n", identification);
  }
}

void RingFIFOBuffer::Snap(void) {
  pringbuffer_t p = &ringbuf;
  pSnap_t s = &ringbufsnap;
#ifdef __RING_BUFFER_SNAP_DEBUG__
  LogDebug("SNAP\n");
  PrintStatus(p, "ringbuffer snap buffer");
#endif
  memcpy((void *)s, (void *)p, sizeof(Snap_t));
#ifdef __RING_BUFFER_SNAP_DEBUG__
  PrintSnapStatus("ringbuffer snap stored");
#endif
}


void RingFIFOBuffer::Discard(void) {
  pringbuffer_t p = &ringbuf;
  pSnap_t s = &ringbufsnap;
#ifdef __RING_BUFFER_DISCARD_DEBUG__
  LogDebug("DISCARD\n");
  PrintStatus((pringbuffer_t)s, "ringbuffer discard stored");
  PrintStatus(p, "ringbuffer discard now");
#endif
  memcpy((void *)p, (void *)s, sizeof(Snap_t));
#ifdef __RING_BUFFER_DISCARD_DEBUG__
  PrintStatus("ringbuffer discard restored");
#endif
}


int RingFIFOBuffer::Write(unsigned char *data, uint32_t length) {
  pringbuffer_t p = &ringbuf;
  int rv = 0, capacity, decrement, first_run_max_length, buffer_size, write_index, frames, bytes, min_capacity, max_frames, max_bytes;
  ulen_t ulen;
  if(length && data) {
    ulen.l = length;
    frames = p->FramesStored;
    bytes = p->NumberOfStoredBytes;
    buffer_size = p->buffer_size;
    write_index = p->write_index;
    first_run_max_length = buffer_size - write_index;
    capacity = buffer_size - bytes;
    decrement = next64aligned(length + 4);
    if(((first_run_max_length >= decrement) && (capacity >= decrement)) || ((first_run_max_length < decrement) && (capacity >= (decrement + first_run_max_length)))) {
      capacity -= decrement;
      p->NumberOfProcessedFrames++;
      frames++;
      min_capacity = p->min_capacity;
      max_frames = p->max_frames;
      max_bytes = p->max_bytes;
      min_capacity = min_capacity <= capacity ? min_capacity : capacity;
      max_frames = max_frames >= frames ? max_frames : frames;
      rv = length;

      if(first_run_max_length < decrement) {
        memset((void*)&p->ptr[write_index], 0, 4); //indicate the jump to the start of the array
        write_index = 0;
        bytes += (decrement + first_run_max_length);
      } else {
        bytes += decrement;
      }
      max_bytes = max_bytes >= bytes ? max_bytes : bytes;
      for (int i=0;i<4;i++) p->ptr[write_index + i] = ulen.b[i];
      memcpy((void *)&p->ptr[write_index+4], (void *)data, length);
      write_index = (int)next64aligned((uint64_t)(write_index + 4 + length));
      p->write_index = write_index >= buffer_size-4 ? 0 : write_index;
      p->NumberOfStoredBytes = bytes;
      p->FramesStored = frames;
      p->min_capacity = min_capacity;
      p->max_frames = max_frames;
      p->max_bytes = max_bytes;
    }
  } else {
    LogDebug("Attempt to write empty message: unsigned char *data = %hhn, uint32_t length = %d\n", (int8_t*) data, length);
  }
  return rv;
}


int RingFIFOBuffer::Read(unsigned char *data) {
  pringbuffer_t p = &ringbuf;
  int rv = 0;
  ulen_t ulen;
  int buffer_size, read_index, frames, bytes;
  buffer_size = p->buffer_size;
  read_index = (int)next64aligned((uint64_t)p->read_index);
  frames = p->FramesStored;
  bytes = p->NumberOfStoredBytes;
  if(frames >0) {
    for (int i=0;i<4;i++) ulen.b[i] = p->ptr[read_index + i];
    if(ulen.l == 0) {
      for (int i=0;i<4;i++) ulen.b[i] = p->ptr[i];
      bytes -= (buffer_size - read_index);
      read_index = 0;
    } else {
      bytes -= next64aligned(ulen.l + 4);
    }
    --frames;
#if(1)
    if(bytes && (frames == 0)) { // leftower bytes in the end must be skipped
#ifdef __DEBUG_RINGBUFFER_CATCH_ZERO_INSERTS__
      PrintStatus(p, "Left over bytes found");
      printf("\n==?== frames = %d, bytes = %d, read_index = %d,  ulen.l = %d, decrement = %lu\n", frames, bytes, read_index, ulen.l, ulen.l ?  next64aligned(ulen.l + 4) : (buffer_size - 1 - read_index));
#endif //__DEBUG_CATCH_ZERO_INSERTS__
      bytes = 0;
    }
#endif //(0)(1)
    memcpy((void *)data, (void *)&p->ptr[read_index+4], ulen.l);
    read_index = (int)next64aligned((uint64_t)(read_index + 4 + ulen.l));
    p->read_index = read_index >= buffer_size-4 ? 0 : read_index;
    p->NumberOfStoredBytes = bytes;
    p->FramesStored = frames;
    rv = ulen.l;
  }
  return rv;
}


int RingFIFOBuffer::GetCapacity(void) {
  pringbuffer_t p = &ringbuf;
  int capacity = p->buffer_size - p->NumberOfStoredBytes;
  if( capacity < 16384) {
    ++p->SlipCounter;
  }
  return capacity;
}

//////////////// testing //////////////////////

uint64_t RingFIFOBuffer::Next64AlignedTest(uint64_t x) { 
  return next64aligned(x);
}

// _eof_


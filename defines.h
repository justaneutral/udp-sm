
#ifndef __DEFINES_H__
#define __DEFINES_H__

#undef __TEST_PUBLISHER_ONLY__
//#define __TEST_PUBLISHER_ONLY__

#undef __WRITE_LITTLE_ENDIAN__
#undef __USE_SHM__

#define __HUGE_PAGES__

#define __ATOMIC__
#define __USE_BUFFERS__

//#undef __CONTROLABLE__
#define __CONTROLABLE__

//#define __TIME_PROFILING__

#define __CACHE_LINE_SIZE__ (64)
#define __SHM_NAME_LENGTH__ (256)

#define LOGNAMSIZ (256)


#define __DATA_BUFFER_SIZE__ (16384)
#define __RING_BUFFER_SIZE__ (128*__DATA_BUFFER_SIZE__)
#define __MESSAGE_TYPE_SHUTDOWN__  (0x01)

#endif // __DEFINES_H__


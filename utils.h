/* SPDX-License-Identifier: BSD-2-Clause */
/* X-SPDX-Copyright-Text: (c) Copyright 2014-2019 Xilinx, Inc. */
#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <inttypes.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <endian.h>

#ifndef MAP_HUGETLB
/* Not always defined in glibc headers.  If the running kernel does not
 * understand this flag it will ignore it and you may not get huge pages.
 * (In that case ef_memreg_alloc() may fail when using packed-stream mode).
 */
# define MAP_HUGETLB  0x40000
#endif


#ifdef __PPC__
# define __ram_buffer_page_size__    (16ll * 1024 * 1024)
#elif defined(__x86_64__) || defined(__i386__)
# define __ram_buffer_page_size__    (2ll * 1024 * 1024)
#elif defined(__aarch64__)
# define __ram_buffer_page_size__    (2ll * 1024 * 1024)
#else
# error "Please define __ram_buffer_page_size__"
#endif

#if(0)
#define TRY(x)                                                  \
  do {                                                          \
    int __rc = (x);                                             \
    if( __rc < 0 ) {                                            \
      fprintf(stderr, "ERROR: TRY(%s) failed\n", #x);           \
      fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__); \
      fprintf(stderr, "ERROR: rc=%d errno=%d (%s)\n",           \
              __rc, errno, strerror(errno));                    \
      abort();                                                  \
    }                                                           \
  } while( 0 )

#define TEST(x)                                                 \
  do {                                                          \
    if( ! (x) ) {                                               \
      fprintf(stderr, "ERROR: TEST(%s) failed\n", #x);          \
      fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__); \
      abort();                                                  \
    }                                                           \
  } while( 0 )
#endif//(0)

#define LogFprintfError(...)   do{ fprintf(stderr, __VA_ARGS__); }while(0)
#define LogFprintfWarning(...)   do{ fprintf(stderr, __VA_ARGS__); }while(0)
#define LogFprintfInfo(...)   do{ fprintf(stdout, __VA_ARGS__); }while(0)
#ifdef NDEBUG
# define LOGV(...)  do{}while(0)
#else
# define LOGV(...)  do{ printf(__VA_ARGS__); }while(0)
//# define LOGV(...)  do{ if(CommonDataPtr->Verbose)  printf(__VA_ARGS__); }while(0)
#endif


#define ROUND_UP(p, align)   (((p)+(align)-1u) & ~((align)-1u))
//#define ROUND_UP(p, align)   ((((p)+(align)-1u)/(align))*align)
#define IS_POW2(n)           (((n) & (n - 1)) == 0)

#define __BUILD_ASSERT_NAME(_x) __BUILD_ASSERT_CPP(_x)
#define __BUILD_ASSERT_CPP(_x)  __BUILD_ASSERT__##_x
#define BUILD_ASSERT(e) \
  typedef char __BUILD_ASSERT_NAME(__LINE__)[(e) ? 1 : -1] \
    __attribute__((unused))



#ifndef SO_TIMESTAMPING
# define SO_TIMESTAMPING                 37
#endif
#ifndef SOF_TIMESTAMPING_TX_HARDWARE
# define SOF_TIMESTAMPING_TX_HARDWARE    (1<<0)
# define SOF_TIMESTAMPING_TX_SOFTWARE    (1<<1)
# define SOF_TIMESTAMPING_RX_HARDWARE    (1<<2)
# define SOF_TIMESTAMPING_RX_SOFTWARE    (1<<3)
# define SOF_TIMESTAMPING_SOFTWARE       (1<<4)
# define SOF_TIMESTAMPING_SYS_HARDWARE   (1<<5)
# define SOF_TIMESTAMPING_RAW_HARDWARE   (1<<6)
#endif


#if __GLIBC__ < 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 9)
# if defined(__BYTE_ORDER) && defined(__BIG_ENDIAN)
#  if __BYTE_ORDER == __BIG_ENDIAN
#   define le16toh(x) __builtin_bswap16(x)
#  else
#   define le16toh(x) (x)
#  endif
# else
#  error "Couldn't determine byte-order on this platform."
# endif
#endif

uint64_t ntoh64(uint64_t v);
uint64_t hton64(uint64_t v);

#endif  /* __UTILS_H__ */

// _eof_


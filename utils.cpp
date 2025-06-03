
#define _GNU_SOURCE 1

#include "utils.h"

uint64_t ntoh64(uint64_t v) {
  uint32_t a = ntohl((uint32_t)(0xffffffff &(v >> 32)));
  uint32_t b = ntohl((uint32_t)(0xffffffff & v));
  uint64_t c = (uint64_t)b;;
  uint64_t d = ((c << 32) & 0xffffffff00000000) | ((uint64_t)a & 0x00000000ffffffff);
  return d;
}

uint64_t hton64(uint64_t v) {
  uint32_t a = ntohl((uint32_t)(0xffffffff &(v >> 32)));
  uint32_t b = ntohl((uint32_t)(0xffffffff & v));
  uint64_t c = (uint64_t)b;;
  uint64_t d = ((c << 32) & 0xffffffff00000000) | ((uint64_t)a & 0x00000000ffffffff);
  return d;
}

// _eof_


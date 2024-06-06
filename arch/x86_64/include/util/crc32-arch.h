#ifndef MIMIK_CRC32_ARCH_H
#define MIMIK_CRC32_ARCH_H

#ifndef MIMIK_CRC32_H
#error "Do not include crc32-arch.h directly. Include crc32.h instead."
#endif

#include <types.h>
#include <util/compiler.h>
#include <util/align.h>

static inline uint32_t crc32c(uint32_t crc, ptr_t buf, size_t len) {
  size_t q;
  uint8_t* pb;
  uint64_t* pq = buf;

  for (q = ALIGNDOWN(len, 8); q > 0; q -= 8)
    ASM("crc32q %1, %%rax" : "+a"(crc) : "m"(*(pq++)));

  pb = (uint8_t*)pq;
  for (len &= 7; len > 0; --len)
    ASM("crc32b %1, %%rax" : "+a"(crc) : "m"(*(pb++)));

  return crc;
}
#define ARCH_CRC32C

#endif

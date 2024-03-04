#ifndef MIMIK_CPU_H
#define MIMIK_CPU_H

#include <types.h>
#include <util/compiler.h>

static inline void
cpuid(uint32_t code, uint32_t out[4]) {
  asm("cpuid"
      : "=a"(out[0]), "=b"(out[1]), "=c"(out[2]), "=d"(out[3])
      : "a"(code));
}

#endif

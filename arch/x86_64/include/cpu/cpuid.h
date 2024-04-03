#ifndef MIMIK_CPU_H
#define MIMIK_CPU_H

#include <types.h>
#include <util/compiler.h>

#define CPUID_STDFN_00000001 0x00000001

struct cpuid_regs {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
};

static inline void
cpuid(uint32_t code, struct cpuid_regs* regs) {
  asm("cpuid"
      : "=a"(regs->eax), "=b"(regs->ebx), "=c"(regs->ecx), "=d"(regs->edx)
      : "a"(code));
}

#endif

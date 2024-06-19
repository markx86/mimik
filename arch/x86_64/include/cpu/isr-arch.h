#ifndef MIMIK_ISR_ARCH_H
#define MIMIK_ISR_ARCH_H

#ifndef MIMIK_ISR_H
#error "Do not include isr-arch.h directly. Include isr.h instead."
#endif

#include <types.h>

enum exception {
  EXCEPTION_DE = 0,
  EXCEPTION_DB,
  EXCEPTION_NMI,
  EXCEPTION_BP,
  EXCEPTION_OF,
  EXCEPTION_BR,
  EXCEPTION_UD,
  EXCEPTION_NM,
  EXCEPTION_DF,
  EXCEPTION_COPROC,
  EXCEPTION_TS,
  EXCEPTION_NP,
  EXCEPTION_SS,
  EXCEPTION_GP,
  EXCEPTION_PF,
  EXCEPTION_RSV1,
  EXCEPTION_MF,
  EXCEPTION_AC,
  EXCEPTION_MC,
  EXCEPTION_XF,
  EXCEPTION_VE,
  EXCEPTION_CP,
  EXCEPTION_RSV2,
  EXCEPTION_RSV3,
  EXCEPTION_RSV4,
  EXCEPTION_RSV5,
  EXCEPTION_RSV6,
  EXCEPTION_RSV7,
  EXCEPTION_HV,
  EXCEPTION_VC,
  EXCEPTION_SX,
  EXCEPTION_RSV8,
  EXCEPTION_MAX
};

typedef uint64_t reg_t;

struct isr_frame {
  reg_t r15;
  reg_t r14;
  reg_t r13;
  reg_t r12;
  reg_t r11;
  reg_t r10;
  reg_t r9;
  reg_t r8;
  reg_t rsi;
  reg_t rdi;
  reg_t rdx;
  reg_t rcx;
  reg_t rbx;
  reg_t rax;
  reg_t rbp;
  uint64_t isr_n;
  uint64_t error;
};

#endif

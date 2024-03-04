#ifndef MIMIK_ISR_H
#define MIMIK_ISR_H

struct isr_frame;

#include <types.h>
#include <cpu/isr-arch.h>

typedef status_t (*isr_t)(struct isr_frame*);

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

void isr_init(void);
void isr_register(enum exception exc, isr_t handler);

#endif

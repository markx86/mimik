#ifndef MIMIK_INT_H
#define MIMIK_INT_H

#include <util/compiler.h>

static inline void
int_enable(void) {
  ASM("sti");
}

static inline void
int_disable(void) {
  ASM("cli");
}

#endif

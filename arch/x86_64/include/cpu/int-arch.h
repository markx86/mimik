#ifndef MIMIK_INT_ARCH_H
#define MIMIK_INT_ARCH_H

#ifndef MIMIK_INT_H
#error "Do not include int-arch.h directly. Include int.h instead."
#endif

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

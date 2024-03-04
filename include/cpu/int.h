#ifndef MIMIK_INT_H
#define MIMIK_INT_H

#include <util/compiler.h>

static inline void int_enable(void) {
	asm("sti");
}

static inline void int_disable(void) {
	asm("cli");
}

#endif

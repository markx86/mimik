#ifndef MIMIK_IO_H
#define MIMIK_IO_H

#include <types.h>
#include <util/compiler.h>

static inline void
io_outb(uint16_t port, uint8_t b) {
  asm("outb %%al, %%dx" : : "al"(b), "dx"(port));
}

static inline uint8_t
io_inb(uint16_t port) {
  uint8_t b;
  asm("inb %%dx, %%al" : "=ax"(b) : "dx"(port));
  return b;
}

#define IO_WAIT() io_outb(0x80, 0)

#endif

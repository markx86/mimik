#ifndef MIMIK_IO_H
#define MIMIK_IO_H

#include <types.h>
#include <util/compiler.h>

static inline void
io_outb(uint16_t port, uint8_t b) {
  ASM("outb %%al, %%dx" : : "a"(b), "d"(port));
}

static inline uint8_t
io_inb(uint16_t port) {
  uint8_t b;
  ASM("inb %%dx, %%al" : "=a"(b) : "d"(port));
  return b;
}

#define IO_WAIT() io_outb(0x80, 0)

#endif

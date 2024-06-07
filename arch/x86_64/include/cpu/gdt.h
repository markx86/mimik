#ifndef MIMIK_GDT_H
#define MIMIK_GDT_H

#ifdef __ASSEMBLER__

#define GDTENTRY(label, base, limit, access, flags) \
  .global _gdt_##label;                             \
  _gdt_##label :;                                   \
  .short(limit) & 0xffff;                           \
  .short(base) & 0xffff;                            \
  .byte((base) >> 16) & 0xff;                       \
  .byte(access);                                    \
  _gdt_##label##_flags :;                           \
  .byte((flags) << 4) | (((limit) >> 16) & 0xf);    \
  .byte((base) >> 24) & 0xff
#define GDTSYSENTRY(label, base, limit, access, flags) \
  GDTENTRY(label, base, limit, access, flags);         \
  .long(base) >> 32;                                   \
  .long 0

#define ACCESS_PRESENT           (1 << 7)
#define ACCESS_PRIVILEGELEVEL(l) (((l) & 0b11) << 5)
#define ACCESS_NORMALSEGMENT     (1 << 4)
#define ACCESS_EXECUTABLE        (1 << 3)
#define ACCESS_READWRITE         (1 << 1)

#define FLAGS_GRANULARITY   (1 << 3)
#define FLAGS_PROTECTEDMODE (1 << 2)
#define FLAGS_LONGMODE      (1 << 1)

#define KERNEL_CS (_gdt_kernel_code - _gdt_null)
#define KERNEL_DS (_gdt_kernel_data - _gdt_null)

#else

#include <types.h>

extern uint8_t _gdt_null[];
extern uint8_t _gdt_kernel_code[];
extern uint8_t _gdt_kernel_data[];
extern uint8_t _gdt_user_code[];
extern uint8_t _gdt_user_data[];

#define GDTOFFSET(s) ((uint64_t)(_gdt_##s - _gdt_null))

#define KERNEL_CS GDTOFFSET(kernel_code)
#define KERNEL_DS GDTOFFSET(kernel_data)
#define USER_CS   GDTOFFSET(user_code)
#define USER_DS   GDTOFFSET(user_data)

#define PL(p) ((p) & 0b11)

void gdt_load(void);

#endif

#endif

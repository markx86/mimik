#include <cpu/isr.h>
#include <cpu/pic.h>
#include <cpu/gdt.h>
#include <util/compiler.h>
#include <util/struct.h>
#include <mem/mem.h>
#include <assert.h>

#define MAX_IDT_ENTRIES 256

struct PACKED idtr {
  uint16_t size;
  uint64_t offset;
};

struct PACKED idt_entry {
  uint16_t offset_low;
  uint16_t segsel;
  uint8_t ist       : 2;
  uint8_t reserved  : 6;
  uint8_t gate_type : 4;
  uint8_t zero      : 1;
  uint8_t dpl       : 2;
  uint8_t present   : 1;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t padding;
};

enum gate_type {
  GATE_INTERRUPT = 0xe,
  GATE_TRAP = 0xf
};

struct idt_entry idt[MAX_IDT_ENTRIES] = {0};
struct idtr idtr = {
  .size = sizeof(idt) - 1,
  .offset = (uint64_t)idt,
};

#define ISR(n) extern void isr##n(void)

/* Exceptions */
ISR(0);
ISR(1);
ISR(2);
ISR(3);
ISR(4);
ISR(5);
ISR(6);
ISR(7);
ISR(8);
ISR(9);
ISR(10);
ISR(11);
ISR(12);
ISR(13);
ISR(14);
ISR(15);
ISR(16);
ISR(17);
ISR(18);
ISR(19);
ISR(20);
ISR(21);
ISR(22);
ISR(23);
ISR(24);
ISR(25);
ISR(26);
ISR(27);
ISR(28);
ISR(29);
ISR(30);
ISR(31);
ISR(32);
ISR(33);
ISR(34);
ISR(35);
ISR(36);
ISR(37);
ISR(38);
ISR(39);
ISR(40);
ISR(41);
ISR(42);
ISR(43);
ISR(44);
ISR(45);
ISR(46);
ISR(47);

static inline void
set_idt_entry(struct idt_entry* entry, ptr_t address, enum gate_type type) {
  entry->offset_low = ((uint64_t)address >> 00) & 0xffff;
  entry->offset_mid = ((uint64_t)address >> 16) & 0xffff;
  entry->offset_high = (uint32_t)((uint64_t)address >> 32) & 0xffffffff;
  entry->segsel = (uint16_t)KERNEL_CS;
  entry->ist = 0;
  entry->gate_type = (uint8_t)(type & 0xf);
  entry->zero = 0;
  entry->present = TRUE;
  entry->dpl = PL(0);
}

#define TRAP(n)      set_idt_entry(&idt[n], (ptr_t)(&isr##n), GATE_TRAP)
#define INTERRUPT(n) set_idt_entry(&idt[n], (ptr_t)(&isr##n), GATE_INTERRUPT)

isr_t isrs[MAX_IDT_ENTRIES];

void
isr_init(void) {
  mem_set(isrs, 0, sizeof(isrs));

  /* register all exceptions */
  TRAP(0);
  TRAP(1);
  INTERRUPT(2);
  TRAP(3);
  TRAP(4);
  TRAP(5);
  TRAP(6);
  TRAP(7);
  TRAP(8);
  TRAP(9);
  TRAP(10);
  TRAP(11);
  TRAP(12);
  TRAP(13);
  TRAP(14);
  TRAP(15);
  TRAP(16);
  TRAP(17);
  TRAP(18);
  TRAP(19);
  TRAP(20);
  TRAP(21);
  TRAP(22);
  TRAP(23);
  TRAP(24);
  TRAP(25);
  TRAP(26);
  TRAP(27);
  TRAP(28);
  TRAP(29);
  TRAP(30);
  TRAP(31);

  /* register all other interrupts */
  INTERRUPT(32);
  INTERRUPT(33);
  INTERRUPT(34);
  INTERRUPT(35);
  INTERRUPT(36);
  INTERRUPT(37);
  INTERRUPT(38);
  INTERRUPT(39);
  INTERRUPT(40);
  INTERRUPT(41);
  INTERRUPT(42);
  INTERRUPT(43);
  INTERRUPT(44);
  INTERRUPT(45);
  INTERRUPT(46);
  INTERRUPT(47);

  ASM("lidt %0" : : "m"(idtr));
}

void
isr_register(size_t irq, isr_t handler, isr_t* old_handler) {
  ASSERT(irq >= 0 && irq < ARRAYLEN(isrs));
  if (old_handler != NULL)
    *old_handler = isrs[irq];
  isrs[irq] = handler;
}

void isr_common(struct isr_frame* frame) {
  isr_t isr;
  size_t n = frame->isr_n;
  ASSERT(n <= UINT8_MAX);
  isr = isrs[n];
  ASSERT(isr && "unhandled interrupt");
  isr(frame);
  pic_ack((uint8_t)n);
}

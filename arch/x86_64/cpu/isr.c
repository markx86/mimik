#include <cpu/isr.h>
#include <cpu/gdt.h>
#include <util/compiler.h>
#include <assert.h>

#define MAX_IDT_ENTRIES 256

struct PACKED idtr {
  uint16_t size;
  uint64_t offset;
};

struct PACKED idt_entry {
  uint16_t offset_low;
  uint16_t segsel;
  uint8_t ist : 2;
  uint8_t reserved : 6;
  uint8_t gate_type : 4;
  uint8_t zero : 1;
  uint8_t dpl : 2;
  uint8_t present : 1;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t padding;
};

enum gate_type {
  GATE_INTERRUPT = 0xE,
  GATE_TRAP = 0xF
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

static inline void
set_idt_entry(struct idt_entry* entry, ptr_t address, enum gate_type type) {
  entry->offset_low = ((uint64_t)address >> 00) & 0xFFFF;
  entry->offset_mid = ((uint64_t)address >> 16) & 0xFFFF;
  entry->offset_high = (uint32_t)((uint64_t)address >> 32) & 0xFFFFFFFF;
  entry->segsel = (uint16_t)KERNEL_CS;
  entry->ist = 0;
  entry->gate_type = (uint8_t)(type & 0xF);
  entry->zero = 0;
  entry->present = TRUE;
  entry->dpl = PL(0);
}

#define TRAP(n) set_idt_entry(&idt[n], (void*)(&isr##n), GATE_TRAP)
#define INTERRUPT(n) set_idt_entry(&idt[n], (void*)(&isr##n), GATE_INTERRUPT)

/* NOTE: make this a list, maybe? */
isr_t isrs[EXCEPTION_MAX] = {0};

void
isr_init(void) {
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

  asm("lidt %0" : : "m"(idtr));
}

void
isr_register(enum exception exc, isr_t handler) {
  ASSERT(exc < EXCEPTION_MAX);
  isrs[exc] = handler;
}

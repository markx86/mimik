#include <arch.h>
#include <cpu/gdt.h>

status_t arch_init(struct bootinfo* bootinfo) {
  gdt_load();
  return SUCCESS;
}

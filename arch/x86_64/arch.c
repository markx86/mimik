#include <arch.h>
#include <cpu/gdt.h>
#include <cpu/pic.h>

status_t
arch_init(struct bootinfo* bootinfo) {
  gdt_load();
  pic_init(&bootinfo->arch);
  pic_mask(0x10);
  return SUCCESS;
}

#ifndef MIMIK_KERNEL_H
#define MIMIK_KERNEL_H

#include <common/initfs.h>
#include <boot/bootinfo.h>
#include <log/log.h>

struct kernel_config {
  struct bootinfo* bootinfo;
  struct initfs fs;
};

extern struct kernel_config kcfg;

#define _(x) extern char _kernel_##x##_start[], _kernel_##x##_end[]

_(paddr);
_(vaddr);
_(text);
_(rodata);
_(data);

#undef _

#define KERNEL_PADDR_START ((addr_t)_kernel_paddr_start)
#define KERNEL_PADDR_END   ((addr_t)_kernel_paddr_end)

#define KERNEL_VADDR_START ((addr_t)_kernel_vaddr_start)
#define KERNEL_VADDR_END   ((addr_t)_kernel_vaddr_end)

#define KERNEL_TEXT_START ((addr_t)_kernel_text_start)
#define KERNEL_TEXT_END   ((addr_t)_kernel_text_end)

#define KERNEL_RODATA_START ((addr_t)_kernel_rodata_start)
#define KERNEL_RODATA_END   ((addr_t)_kernel_rodata_end)

#define KERNEL_DATA_START ((addr_t)_kernel_data_start)
#define KERNEL_DATA_END   ((addr_t)_kernel_data_end)

#define KERNEL_HEAP_START 0xffff800000000000

#endif

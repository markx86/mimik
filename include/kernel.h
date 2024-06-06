#ifndef MIMIK_KERNEL_H
#define MIMIK_KERNEL_H

#include <boot/bootinfo.h>
#include <fs/initfs.h>
#include <log/log.h>

struct kernel_config {
  struct bootinfo* bootinfo;
  struct initfs fs;
};

extern struct kernel_config kcfg;

extern char _kernel_paddr_start[];
#define KERNEL_START_PADDR ((addr_t)_kernel_paddr_start)
extern char _kernel_paddr_end[];
#define KERNEL_END_PADDR ((addr_t)_kernel_paddr_end)

extern char _kernel_vaddr_start[];
#define KERNEL_START_VADDR ((addr_t)_kernel_vaddr_start)
extern char _kernel_vaddr_end[];
#define KERNEL_END_VADDR ((addr_t)_kernel_vaddr_end)

#define KERNEL_HEAP_START 0xffff800000000000

#endif

#ifndef MIMIK_LAYOUT_H
#define MIMIK_LAYOUT_H

#include <types.h>
#include <assert.h>

extern const char _entry_paddr_start[0], _entry_vaddr_start[0];
extern const char _entry_paddr_end[0], _entry_vaddr_end[0];

#define _(x) extern const char _kernel_##x##_start[0], _kernel_##x##_end[0]

_(paddr);
_(vaddr);
_(text);
_(rodata);
_(data);

#undef _

#define LAYOUT_HIGHER_HALF HIGHER_HALF

#define LAYOUT_ENTRY_PADDR_START ((addr_t)_entry_vaddr_start)
#define LAYOUT_ENTRY_PADDR_END   ((addr_t)_entry_vaddr_end)

#define LAYOUT_ENTRY_VADDR_START ((addr_t)_entry_vaddr_start)
#define LAYOUT_ENTRY_VADDR_END   ((addr_t)_entry_vaddr_end)

#define LAYOUT_KERNEL_PADDR_START ((addr_t)_kernel_paddr_start)
#define LAYOUT_KERNEL_PADDR_END   ((addr_t)_kernel_paddr_end)

#define LAYOUT_KERNEL_VADDR_START ((addr_t)_kernel_vaddr_start)
#define LAYOUT_KERNEL_VADDR_END   ((addr_t)_kernel_vaddr_end)

#define LAYOUT_TEXT_START ((addr_t)_kernel_text_start)
#define LAYOUT_TEXT_END   ((addr_t)_kernel_text_end)

#define LAYOUT_RODATA_START ((addr_t)_kernel_rodata_start)
#define LAYOUT_RODATA_END   ((addr_t)_kernel_rodata_end)

#define LAYOUT_DATA_START ((addr_t)_kernel_data_start)
#define LAYOUT_DATA_END   ((addr_t)_kernel_data_end)

#define LAYOUT_HEAP_START 0xffff800000000000

#define LAYOUT_PHYSMAP_BASE 0xfffff80000000000

static inline addr_t
layout_vaddr_to_paddr(addr_t vaddr) {
  ASSERT(vaddr >= LAYOUT_HEAP_START);
  return vaddr - LAYOUT_PHYSMAP_BASE;
}

static inline addr_t
layout_paddr_to_vaddr(addr_t paddr) {
  ASSERT(paddr < LAYOUT_HEAP_START);
  return paddr + LAYOUT_PHYSMAP_BASE;
}

#endif

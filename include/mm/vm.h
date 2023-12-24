#ifndef MIMIK_VM_H
#define MIMIK_VM_H

#include <bootinfo.h>
#include <mm/page.h>

enum vm_map_flags {
  VM_MAP_STRICT = 1 << 0,
  VM_MAP_USER = 1 << 1,
  VM_MAP_WRITABLE = 1 << 2,
  VM_MAP_EXECUTABLE = 1 << 3,
};

void vm_init(void);
void vm_flush_tlb(void);

status_t vm_map_pages(
    ptr_t table,
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    enum vm_map_flags flags);
#define vm_map_page(table, paddr, vaddr_hint, flags) \
  vm_map_pages(table, paddr, 1, vaddr_hint, flags)
#define vm_map_range(table, paddr_start, paddr_end, vaddr_hint, flags) \
  vm_map_pages(                                                        \
      table,                                                           \
      paddr_start,                                                     \
      PAGES(paddr_end - paddr_start),                                  \
      vaddr_hint,                                                      \
      flags)
status_t vm_map_kernel_pages(
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    enum vm_map_flags flags);
#define vm_map_kernel_page(paddr, vaddr_hint, flags) \
  vm_map_kernel_pages(paddr, 1, vaddr_hint, flags)
#define vm_map_kernel_range(paddr_start, paddr_end, vaddr_hint, flags) \
  vm_map_kernel_pages(                                                 \
      paddr_start,                                                     \
      PAGES(paddr_end - paddr_start),                                  \
      vaddr_hint,                                                      \
      flags)

void vm_unmap_pages(ptr_t table, addr_t vaddr_start, size_t pages);
#define vm_unmap_page(table, vaddr) vm_unmap_pages(table, vaddr, 1)
#define vm_unmap_range(table, vaddr_start, vaddr_end) \
  vm_unmap_pages(table, vaddr_start, PAGES(vaddr_end - vaddr_start))
void vm_unmap_kernel_pages(addr_t vaddr_start, size_t pages);
#define vm_unmap_kernel_page(vaddr) vm_unmap_kernel_pages(vaddr, 1)
#define vm_unmap_kernel_range(vaddr_start, vaddr_end) \
  vm_unmap_kernel_range(vaddr_start, PAGES(vaddr_end - vaddr_start))

status_t vm_vaddr_to_paddr(ptr_t table, addr_t vaddr, addr_t* paddr);
status_t vm_kernel_vaddr_to_paddr(addr_t vaddr, addr_t* paddr);

#endif

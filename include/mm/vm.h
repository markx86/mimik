#ifndef MIMIK_VM_H
#define MIMIK_VM_H

#include <boot/bootinfo.h>
#include <mem/page.h>

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
status_t vm_map_bytes(
    ptr_t table,
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    enum vm_map_flags flags);
#define vm_map_page(table, paddr, vaddr_hint, flags) \
  vm_map_pages(table, paddr, 1, vaddr_hint, flags)
#define vm_map_range(table, paddr_start, paddr_end, vaddr_hint, flags) \
  vm_map_bytes(table, paddr_start, paddr_end - paddr_start, vaddr_hint, flags)
status_t vm_kmap_pages(
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    enum vm_map_flags flags);
status_t vm_kmap_bytes(
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    enum vm_map_flags flags);
#define vm_kmap_page(paddr, vaddr_hint, flags) \
  vm_kmap_pages(paddr, 1, vaddr_hint, flags)
#define vm_kmap_range(paddr_start, paddr_end, vaddr_hint, flags) \
  vm_kmap_bytes(paddr_start, paddr_end - paddr_start, vaddr_hint, flags)

size_t vm_unmap_pages(ptr_t table, addr_t vaddr_start, size_t pages);
size_t vm_unmap_bytes(ptr_t table, addr_t vaddr_start, size_t bytes);
#define vm_unmap_page(table, vaddr) vm_unmap_pages(table, vaddr, 1)
#define vm_unmap_range(table, vaddr_start, vaddr_end) \
  vm_unmap_bytes(table, vaddr_start, vaddr_end - vaddr_start)
size_t vm_kunmap_pages(addr_t vaddr_start, size_t pages);
size_t vm_kunmap_bytes(addr_t vaddr_start, size_t bytes);
#define vm_kunmap_page(vaddr) vm_kunmap_pages(vaddr, 1)
#define vm_kunmap_range(vaddr_start, vaddr_end) \
  vm_kunmap_bytes(vaddr_start, vaddr_end - vaddr_start)

status_t vm_vaddr_to_paddr(ptr_t table, addr_t vaddr, addr_t* paddr);
status_t vm_kvaddr_to_paddr(addr_t vaddr, addr_t* paddr);

#endif

#ifndef MIMIK_VM_H
#define MIMIK_VM_H

#include <boot/bootinfo.h>
#include <mem/page.h>

#define VM_FLAG_READ       0
#define VM_FLAG_STRICT     (1 << 0)
#define VM_FLAG_USER       (1 << 1)
#define VM_FLAG_WRITABLE   (1 << 2)
#define VM_FLAG_EXECUTABLE (1 << 3)

void vm_init(void);
void vm_flush_tlb(void);

status_t vm_map_pages(
    ptr_t table,
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    int flags);
status_t vm_map_bytes(
    ptr_t table,
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    int flags);
#define vm_map_page(table, paddr, vaddr_hint, flags) \
  vm_map_pages(table, paddr, 1, vaddr_hint, flags)
#define vm_map_range(table, paddr_start, paddr_end, vaddr_hint, flags) \
  vm_map_bytes(table, paddr_start, paddr_end - paddr_start, vaddr_hint, flags)

status_t vmk_map_pages(
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    int flags);
status_t vmk_map_bytes(
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    int flags);
#define vmk_map_page(paddr, vaddr_hint, flags) \
  vmk_map_pages(paddr, 1, vaddr_hint, flags)
#define vmk_map_range(paddr_start, paddr_end, vaddr_hint, flags) \
  vmk_map_bytes(paddr_start, paddr_end - paddr_start, vaddr_hint, flags)

size_t vm_unmap_pages(ptr_t table, addr_t vaddr_start, size_t pages);
size_t vm_unmap_bytes(ptr_t table, addr_t vaddr_start, size_t bytes);
#define vm_unmap_page(table, vaddr) vm_unmap_pages(table, vaddr, 1)
#define vm_unmap_range(table, vaddr_start, vaddr_end) \
  vm_unmap_bytes(table, vaddr_start, vaddr_end - vaddr_start)

size_t vmk_unmap_pages(addr_t vaddr_start, size_t pages);
size_t vmk_unmap_bytes(addr_t vaddr_start, size_t bytes);
#define vmk_unmap_page(vaddr) vm_kunmap_pages(vaddr, 1)
#define vmk_unmap_range(vaddr_start, vaddr_end) \
  vmk_unmap_bytes(vaddr_start, vaddr_end - vaddr_start)

status_t vm_vaddr_to_paddr(ptr_t table, addr_t vaddr, addr_t* paddr);
status_t vmk_vaddr_to_paddr(addr_t vaddr, addr_t* paddr);

status_t vm_set_backing(ptr_t table, addr_t vaddr, addr_t paddr);
status_t vmk_set_backing(addr_t vaddr, addr_t paddr);

#define vm_reserve_pages(table, vaddr_hint, pages, flags) \
  vm_map_pages(table, (addr_t) - BYTES(pages), pages, vaddr_hint, flags)
#define vmk_reserve_pages(vaddr_hint, pages, flags) \
  vmk_map_pages((addr_t) - BYTES(pages), pages, vaddr_hint, flags)

  
status_t vm_flag_pages(ptr_t table, addr_t vaddr_start, size_t pages, int flags);
#define vm_flag_range(table, vaddr_start, vaddr_end, flags) \
  vm_flag_pages(table, vaddr_start, PAGES(vaddr_end - vaddr_start), flags)

status_t vmk_flag_pages(addr_t vaddr_start, size_t pages, int flags);
#define vmk_flag_range(vaddr_start, vaddr_end, flags) \
  vmk_flag_pages(vaddr_start, PAGES(vaddr_end - vaddr_start), flags)

#endif

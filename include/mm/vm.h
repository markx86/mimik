#ifndef MIMIK_VM_H
#define MIMIK_VM_H

#include <boot/bootinfo.h>
#include <mem/page.h>
#include <mm/vm-arch.h>
#include <assert.h>

#define VM_FLAG_READ  0
#define VM_FLAG_USER  (1 << 0)
#define VM_FLAG_WRITE (1 << 1)
#define VM_FLAG_EXEC  (1 << 2)

struct pt;

void vm_init(void);

status_t vm_map_pages(
    struct pt* table,
    addr_t paddr,
    size_t pages,
    addr_t vaddr,
    flags_t flags);

static inline status_t
vm_map_bytes(
    struct pt* table,
    addr_t paddr,
    size_t bytes,
    addr_t vaddr,
    flags_t flags) {
  addr_t aligned_paddr;
  ASSERT(PAGEOFFSET(paddr) == PAGEOFFSET(vaddr));
  aligned_paddr = PAGEALIGNDOWN(paddr);
  bytes = (paddr + bytes) - aligned_paddr;
  vaddr = PAGEALIGNDOWN(vaddr);
  return vm_map_pages(table, aligned_paddr, PAGES(bytes), vaddr, flags);
}

static inline status_t
vm_map_page(struct pt* table, addr_t paddr, addr_t vaddr, flags_t flags) {
  return vm_map_pages(table, paddr, 1, vaddr, flags);
}

static inline status_t
vm_map_range(
    struct pt* table,
    addr_t paddr_start,
    addr_t paddr_end,
    addr_t vaddr,
    flags_t flags) {
  return vm_map_bytes(
      table,
      paddr_start,
      paddr_end - paddr_start,
      vaddr,
      flags);
}

status_t vmk_map_pages(addr_t paddr, size_t pages, addr_t vaddr, flags_t flags);

static inline status_t
vmk_map_bytes(addr_t paddr, size_t bytes, addr_t vaddr, flags_t flags) {
  addr_t aligned_paddr;
  ASSERT(PAGEOFFSET(paddr) == PAGEOFFSET(vaddr));
  aligned_paddr = PAGEALIGNDOWN(paddr);
  bytes = (paddr + bytes) - aligned_paddr;
  vaddr = PAGEALIGNDOWN(vaddr);
  return vmk_map_pages(aligned_paddr, PAGES(bytes), vaddr, flags);
}

static inline status_t
vmk_map_page(addr_t paddr, addr_t vaddr, flags_t flags) {
  return vmk_map_pages(paddr, 1, vaddr, flags);
}

static inline status_t
vmk_map_range(
    addr_t paddr_start,
    addr_t paddr_end,
    addr_t vaddr,
    flags_t flags) {
  return vmk_map_bytes(paddr_start, paddr_end - paddr_start, vaddr, flags);
}

void vm_unmap_pages(struct pt* table, addr_t vaddr, size_t pages);

static inline void
vm_unmap_bytes(struct pt* table, addr_t vaddr, size_t bytes) {
  addr_t aligned_vaddr = PAGEALIGNDOWN(vaddr);
  bytes = (vaddr + bytes) - aligned_vaddr;
  vm_unmap_pages(table, aligned_vaddr, PAGES(bytes));
}

static inline void
vm_unmap_page(struct pt* table, addr_t vaddr) {
  vm_unmap_pages(table, vaddr, 1);
}

static inline void
vm_unmap_range(struct pt* table, addr_t vaddr_start, addr_t vaddr_end) {
  vm_unmap_bytes(table, vaddr_start, vaddr_end - vaddr_start);
}

void vmk_unmap_pages(addr_t vaddr, size_t pages);

static inline void
vmk_unmap_bytes(addr_t vaddr, size_t bytes) {
  addr_t aligned_vaddr = PAGEALIGNDOWN(vaddr);
  bytes = (vaddr + bytes) - aligned_vaddr;
  vmk_unmap_pages(aligned_vaddr, PAGES(bytes));
}

static inline void
vmk_unmap_page(addr_t vaddr) {
  vmk_unmap_pages(vaddr, 1);
}

static inline void
vmk_unmap_range(addr_t vaddr_start, addr_t vaddr_end) {
  vmk_unmap_bytes(vaddr_start, vaddr_end - vaddr_start);
}

status_t vm_resolve_vaddr(struct pt* table, addr_t vaddr, addr_t* paddr);
status_t vmk_resolve_vaddr(addr_t vaddr, addr_t* paddr);

status_t vm_map_edit(
    struct pt* table,
    addr_t vaddr,
    addr_t new_paddr,
    flags_t new_flags);

static inline status_t
vm_map_edit_flags(struct pt* table, addr_t vaddr, flags_t new_flags) {
  return vm_map_edit(table, vaddr, ADDR_DUMMY, new_flags);
}

static inline status_t
vm_map_edit_paddr(struct pt* table, addr_t vaddr, addr_t new_paddr) {
  return vm_map_edit(table, vaddr, new_paddr, FLAGS_DUMMY);
}

status_t vmk_map_edit(addr_t vaddr, addr_t new_paddr, flags_t new_flags);

static inline status_t
vmk_map_edit_flags(addr_t vaddr, flags_t new_flags) {
  return vmk_map_edit(vaddr, ADDR_DUMMY, new_flags);
}

static inline status_t
vmk_map_edit_paddr(addr_t vaddr, addr_t new_paddr) {
  return vmk_map_edit(vaddr, new_paddr, FLAGS_DUMMY);
}

static inline status_t
vm_reserve_pages(struct pt* table, addr_t vaddr, size_t pages, flags_t flags) {
  return vm_map_pages(table, ADDR_DUMMY, pages, vaddr, flags);
}

static inline status_t
vmk_reserve_pages(addr_t vaddr, size_t pages, flags_t flags) {
  return vmk_map_pages(ADDR_DUMMY, pages, vaddr, flags);
}

static inline status_t
vm_map_edit_flags_range(
    struct pt* table,
    addr_t vaddr_start,
    addr_t vaddr_end,
    flags_t new_flags) {
  addr_t vaddr;
  status_t res;
  for (vaddr = vaddr_start; vaddr < vaddr_end; vaddr += PAGE_SIZE) {
    res = vm_map_edit_flags(table, vaddr, new_flags);
    if (ISERROR(res))
      return res;
  }
  return SUCCESS;
}

static inline status_t
vmk_map_edit_flags_range(
    addr_t vaddr_start,
    addr_t vaddr_end,
    flags_t new_flags) {
  addr_t vaddr;
  status_t res;
  vaddr_start = PAGEALIGNDOWN(vaddr_start);
  vaddr_end = PAGEALIGNUP(vaddr_end);
  for (vaddr = vaddr_start; vaddr < vaddr_end; vaddr += PAGE_SIZE) {
    res = vmk_map_edit_flags(vaddr, new_flags);
    if (ISERROR(res))
      return res;
  }
  return SUCCESS;
}

#endif

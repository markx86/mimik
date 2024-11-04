#include <mm/vm.h>
#include <mm/pm.h>
#include <mem/layout.h>
#include <mem/mem.h>
#include <cpu/isr.h>
#include <kernel.h>
#include <assert.h>

#define PT_LAST_LEVEL       (PT_LEVELS - 1)
#define VADDR_PAGE_IDX_MASK ((1 << VADDR_PAGE_IDX_BITS) - 1)

#define ISFLAGSET(flags, flag)   ((flags & VM_FLAG_##flag) != 0)
#define ISFLAGUNSET(flags, flag) ((flags & VM_FLAG_##flag) == 0)

#define ISKVADDR(a) ((a) >= LAYOUT_HEAP_START)
#define PTEFLAG(b)  ((pte_t)1 << b)

#define PTE_FLAG_PRESENT PTEFLAG(PTE_BIT_PRESENT)
#define PTE_FLAG_WRITE   PTEFLAG(PTE_BIT_WRITE)
#define PTE_FLAG_USER    PTEFLAG(PTE_BIT_USER)
#if defined(PTE_BIT_EXECUTE) /* PTE_BIT_EXECUTE */
#define PTE_FLAG_EXECUTE PTEFLAG(PTE_BIT_EXECUTE)
#elif defined(PTE_BIT_NOEXECUTE) /* PTE_BIT_NOEXECUTE */
#define PTE_FLAG_NOEXECUTE PTEFLAG(PTE_BIT_NOEXECUTE)
#endif

struct pt {
  pte_t e[PT_LENGTH];
};
STATICASSERT(
    sizeof(struct pt) <= PAGE_SIZE,
    "a page table must not be larger than a page");

static struct pt* kpt;

static inline void
pte_change_flag(pte_t* pte, pte_t mask, uint32_t value) {
  if (value)
    *pte |= mask;
  else
    *pte &= ~mask;
}

static inline void
pte_set_flags(pte_t* pte, flags_t flags) {
  /* ensure flags are valid */
  ASSERT(flags != FLAGS_DUMMY);

  pte_change_flag(pte, PTE_FLAG_PRESENT, 1);
  pte_change_flag(pte, PTE_FLAG_WRITE, flags & VM_FLAG_WRITE);
  pte_change_flag(pte, PTE_FLAG_USER, flags & VM_FLAG_USER);
#if defined(PTE_BIT_EXECUTE)
  pte_change_bit(pte, PTE_FLAG_EXECUTE, flags & VM_FLAG_EXEC);
#elif defined(PTE_BIT_NOEXECUTE)
  pte_change_flag(pte, PTE_FLAG_NOEXECUTE, !(flags & VM_FLAG_EXEC));
#endif
}

static inline void
vaddr_to_indices(addr_t vaddr, size_t indices[PT_LEVELS]) {
  size_t i;

  vaddr >>= PAGE_SHIFT;
  for (i = PT_LEVELS; i > 0; --i) {
    indices[i - 1] = vaddr & VADDR_PAGE_IDX_MASK;
    vaddr >>= VADDR_PAGE_IDX_BITS;
  }
}

static inline addr_t
indices_to_vaddr(size_t indices[PT_LEVELS]) {
  size_t i;
  addr_t vaddr = 0;

  for (i = 0; i < PT_LEVELS; ++i) {
    vaddr <<= VADDR_PAGE_IDX_BITS;
    vaddr |= indices[i];
  }

  return vaddr << PAGE_SHIFT;
}

static inline size_t
get_pages_per_level(size_t level) {
  size_t pages = 1;

  level = PT_LEVELS - level;
  for (; level > 0; --level)
    pages *= PT_LENGTH;

  return pages;
}

/* TODO: make this a part of layout.h/layout.c */
static inline bool_t
is_physmap_vaddr(addr_t va) {
  size_t total_mem = pm_get_total_memory();
  return va >= LAYOUT_PHYSMAP_BASE && va < LAYOUT_PHYSMAP_BASE + total_mem;
}

static void
page_fault_handler(struct isr_frame* frame) {
  addr_t fault, vaddr, paddr;
  status_t res;
  bool_t is_kernel;

  ASSERT(frame->isr_n == EXCEPTION_PF);
  ASSERT(K.inited.vm);

  /* TODO: put this in its own function? */
  is_kernel =
      vm_get_current_root_pt_paddr() == layout_vaddr_to_paddr((addr_t)kpt);

  fault = vm_get_fault_address();
  if (is_kernel) {
    if (is_physmap_vaddr(fault)) {
      vaddr = PAGEALIGNDOWN(fault);
      paddr = layout_vaddr_to_paddr(vaddr);
      /*
       * FIXME: do not blindly map everything as RW-, check if it should be RO
       *        or not mapped at all
       */
      res = vmk_map_page(paddr, vaddr, VM_FLAG_READ | VM_FLAG_WRITE);
      if (res == SUCCESS)
        return;
    }
  }

  UNREACHABLE();
}

void
vm_init(void) {
  struct pt* pt;
  size_t level, idx;
  size_t indices_lo[PT_LEVELS], indices_hi[PT_LEVELS];
  addr_t vaddr_lo, vaddr_hi, kpt_paddr;
  addr_t paddr_lo, paddr_hi, vaddr;
  status_t res;
  pte_t* pte;

  ASSERT(!K.inited.vm);

  kpt_paddr = vm_get_current_root_pt_paddr();
  kpt = (struct pt*)(kpt_paddr + HIGHER_HALF);

  /* unmap lower addresses */
  {
    pt = kpt;

    vaddr_lo = LAYOUT_ENTRY_PADDR_START;
    vaddr_to_indices(vaddr_lo, indices_lo);

    vaddr_hi = LAYOUT_KERNEL_VADDR_START;
    vaddr_to_indices(vaddr_hi, indices_hi);

    for (level = 0; level < PT_LAST_LEVEL; ++level) {
      idx = indices_lo[level];
      if (idx == indices_hi[level])
        continue;
      pte = &pt->e[idx];
      pt = (struct pt*)(pte_get_paddr(*pte) + HIGHER_HALF);
      *pte = 0;
    }

    vm_flush_tlb();
  }

  /* map the page tables in the physmap region */
  {
    paddr_lo = PAGEALIGNUP(K.bootinfo->data_paddr_end);
    paddr_hi = PAGEALIGNUP(K.bootinfo->first_free_paddr);
    vaddr_to_indices(paddr_hi, indices_hi);
    paddr_hi += BYTES(PT_LENGTH - indices_hi[PT_LAST_LEVEL]);
    ASSERT(paddr_hi > paddr_lo);

    vaddr = layout_paddr_to_vaddr(paddr_lo);

    res =
        vmk_map_range(paddr_lo, paddr_hi, vaddr, VM_FLAG_READ | VM_FLAG_WRITE);
    ASSERT(res == SUCCESS);
  }

  kpt = (struct pt*)layout_paddr_to_vaddr(kpt_paddr);
  isr_register(EXCEPTION_PF, &page_fault_handler, NULL);

  LOGSUCCESS("virtual memory manager initialized");
  K.inited.vm = TRUE;
}

static inline addr_t
paddr_to_vaddr(addr_t paddr) {
  if (LIKELY(K.inited.vm))
    return layout_paddr_to_vaddr(paddr);
  else
    return paddr + HIGHER_HALF;
}

static status_t
recurse_map(
    struct pt* pt,
    size_t level,
    size_t indices[PT_LEVELS],
    addr_t paddr,
    size_t pages,
    flags_t flags) {
  struct pt* next_pt;
  pte_t* pte;
  size_t pages_mapped, *idx;
  addr_t pte_paddr;
  bool_t pte_not_present;
  status_t res;

  ASSERT(level < PT_LEVELS);
  ASSERT(pt != NULL && (addr_t)pt >= LAYOUT_PHYSMAP_BASE);

  idx = &indices[level];
  ASSERT(*idx < PT_LENGTH);

  pages_mapped = 0;
  for (; *idx < PT_LENGTH; ++(*idx)) {
    pte = &pt->e[*idx];
    pte_not_present = !(*pte & PTE_FLAG_PRESENT);

    if (level < PT_LAST_LEVEL) {
      if (pte_not_present) {
        pte_paddr = pm_request_page();
        pte_set_paddr(pte, pte_paddr);
        pte_set_flags(pte, flags);
      } else
        pte_paddr = pte_get_paddr(*pte);

      next_pt = (struct pt*)paddr_to_vaddr(pte_paddr);
      if (pte_not_present)
        mem_set(next_pt, 0, sizeof(struct pt));
      res = recurse_map(next_pt, level + 1, indices, paddr, pages, flags);
      if (ISERROR(res))
        return res;

      pages_mapped += (size_t)res;
    } else {
      if (UNLIKELY(pte_not_present == FALSE))
        return -EOVERLAP;
      else {
        pte_set_flags(pte, flags);
        pte_set_paddr(pte, paddr);
      }

      paddr += SIZE_4KB;
      ++pages_mapped;
    }

    if (pages_mapped >= pages)
      break;
    else if (level < PT_LAST_LEVEL)
      *(idx + 1) = 0;
  }

  ASSERT(pages_mapped <= SSIZE_MAX);
  return (ssize_t)pages_mapped;
}

status_t
vm_map_pages(
    struct pt* table,
    addr_t paddr,
    size_t pages,
    addr_t vaddr,
    flags_t flags) {
  status_t res;
  size_t indices[PT_LEVELS];

  ASSERT(table != NULL);
  /* ensure the paddr is page aligned */
  ASSERT(ISPAGEALIGNED(paddr) || paddr == ADDR_DUMMY);
  ASSERT(ISPAGEALIGNED(vaddr));
  /* ensure we're actually doing something */
  ASSERT(pages > 0);
  ASSERT(vaddr != 0);
  /* ensure the paddr is within bounds (exception for dummy address) */
  ASSERT(paddr < pm_get_total_memory() || paddr == ADDR_DUMMY);
  /* ensure we can return the number of pages mapped */
  ASSERT(pages <= SSIZE_MAX);

  vaddr_to_indices(vaddr, indices);

  res = recurse_map(table, 0, indices, paddr, pages, flags);
  if (ISERROR(res))
    return res;
  ASSERT((size_t)res == pages);
  return SUCCESS;
}

status_t
vmk_map_pages(addr_t paddr, size_t pages, addr_t vaddr, flags_t flags) {
  ASSERT(ISFLAGUNSET(flags, USER));
  ASSERT(vaddr != 0);
  ASSERT(ISKVADDR(vaddr));
  return vm_map_pages(kpt, paddr, pages, vaddr, flags);
}

static size_t
recurse_unmap(
    struct pt* pt,
    size_t level,
    size_t indices[PT_LEVELS],
    size_t pages) {
  struct pt* next_pt;
  size_t unmapped_pages, pages_per_level, *idx;
  addr_t pte_paddr, pte_vaddr;
  pte_t* pte;

  ASSERT(level < PT_LEVELS);

  idx = &indices[level];
  ASSERT(*idx < PT_LENGTH);

  unmapped_pages = pages_per_level = 0;
  for (; *idx < PT_LENGTH; ++(*idx)) {
    pte = &pt->e[*idx];
    if (*pte & PTE_FLAG_PRESENT) {
      if (level == PT_LAST_LEVEL) {
        pte_vaddr = indices_to_vaddr(indices);
        *pte = 0;
        vm_invalidate_page(pte_vaddr);
        ++unmapped_pages;
      } else {
        pte_paddr = pte_get_paddr(*pte);
        next_pt = (struct pt*)paddr_to_vaddr(pte_paddr);
        unmapped_pages += recurse_unmap(next_pt, level + 1, indices, pages);
        /* TODO: check if pt is empty and release it to the page allocator if it
         * is */
        if (unmapped_pages >= pages)
          break;
      }
    } else {
      if (UNLIKELY(pages_per_level == 0))
        pages_per_level = get_pages_per_level(level);
      unmapped_pages += pages_per_level;
      /* clamp value to the requested one */
      if (unmapped_pages > pages) {
        unmapped_pages = pages;
        break;
      }
    }
  }

  return unmapped_pages;
}

void
vm_unmap_pages(struct pt* table, addr_t vaddr, size_t pages) {
  size_t unmapped, indices[PT_LEVELS];

  ASSERT(table != NULL);
  /* ensure the vaddr is aligned */
  ASSERT(ISPAGEALIGNED(vaddr));
  /* ensure we're actually doing something */
  ASSERT(pages > 0);
  /* forbid unmapping physmap region */
  ASSERT(
      vaddr + BYTES(pages) <= LAYOUT_PHYSMAP_BASE ||
      vaddr >= LAYOUT_HIGHER_HALF);

  vaddr_to_indices(vaddr, indices);

  unmapped = recurse_unmap(table, 0, indices, pages);
  ASSERT(unmapped == pages);
}

void
vmk_unmap_pages(addr_t vaddr, size_t pages) {
  ASSERT(ISKVADDR(vaddr));
  return vm_unmap_pages(kpt, vaddr, pages);
}

status_t
vm_resolve_vaddr(struct pt* table, addr_t vaddr, addr_t* paddr) {
  struct pt* pt;
  size_t level, idx, indices[PT_LEVELS];
  addr_t pte_paddr;
  pte_t pte;

  /* ensure pt is valid */
  ASSERT(table != NULL);
  /* ensure addresses are page aligned */
  ASSERT(ISPAGEALIGNED(vaddr));
  /* ensure paddr is a valid ptr */
  ASSERT(paddr != NULL);

  vaddr_to_indices(vaddr, indices);

  pt = table;
  for (level = 0; level < PT_LEVELS; ++level) {
    idx = indices[level];
    ASSERT(idx < PT_LENGTH);

    pte = pt->e[idx];
    if (!(pte & PTE_FLAG_PRESENT))
      return -ENOMAP;

    pte_paddr = pte_get_paddr(pte);
    if (level == PT_LAST_LEVEL) {
      *paddr = pte_paddr;
      return SUCCESS;
    }

    pt = (struct pt*)paddr_to_vaddr(pte_paddr);
  }

  UNREACHABLE();
}

status_t
vmk_resolve_vaddr(addr_t vaddr, addr_t* paddr) {
  ASSERT(ISKVADDR(vaddr));
  return vm_resolve_vaddr(kpt, vaddr, paddr);
}

status_t
vm_map_edit(
    struct pt* table,
    addr_t vaddr,
    addr_t new_paddr,
    flags_t new_flags) {
  struct pt* pt;
  size_t level, idx, indices[PT_LEVELS];
  addr_t pte_paddr;
  pte_t pte;

  /* ensure pt is valid */
  ASSERT(table != NULL);
  /* ensure addresses are page aligned (except for dummy address) */
  ASSERT(ISPAGEALIGNED(vaddr));
  ASSERT(new_paddr == ADDR_DUMMY || ISPAGEALIGNED(new_paddr));
  /* ensure paddr is within bounds (except for dummy address) */
  ASSERT(new_paddr == ADDR_DUMMY || new_paddr < pm_get_total_memory());

  /*
   * if we initialized the kernel, ensure that nobody can change the
   * kernel region permissions
   */
  if (K.inited.kernel)
    ASSERT(vaddr < LAYOUT_ENTRY_VADDR_START || vaddr > LAYOUT_KERNEL_VADDR_END);

  vaddr_to_indices(vaddr, indices);

  pt = table;
  for (level = 0; level < PT_LEVELS; ++level) {
    idx = indices[level];
    ASSERT(idx < PT_LENGTH);

    pte = pt->e[idx];

    if (!(pte & PTE_FLAG_PRESENT))
      return -ENOMAP;

    if (level == PT_LAST_LEVEL) {
      if (new_flags != FLAGS_DUMMY)
        pte_set_flags(&pte, new_flags);
      if (new_paddr != ADDR_DUMMY)
        pte_set_paddr(&pte, new_paddr);
      vm_invalidate_page(vaddr);
      pt->e[idx] = pte;
      return SUCCESS;
    } else {
      pte_paddr = pte_get_paddr(pte);
      pt = (struct pt*)paddr_to_vaddr(pte_paddr);
    }
  }

  UNREACHABLE();
}

status_t
vmk_map_edit(addr_t vaddr, addr_t new_paddr, flags_t new_flags) {
  ASSERT(ISKVADDR(vaddr));
  return vm_map_edit(kpt, vaddr, new_paddr, new_flags);
}

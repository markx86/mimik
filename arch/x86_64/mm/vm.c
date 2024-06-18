#include <mm/vm.h>
#include <mm/pm.h>
#include <mem/page.h>
#include <mem/mem.h>
#include <log/log.h>
#include <util/compiler.h>
#include <types.h>
#include <kernel.h>
#include <assert.h>

union pte {
  struct PACKED {
    uint64_t present         : 1;
    uint64_t writable        : 1;
    uint64_t user_accessible : 1;
    uint64_t write_through   : 1;
    uint64_t cache_disable   : 1;
    uint64_t accessed        : 1;
    uint64_t dirty           : 1;
    uint64_t is_big          : 1;
    uint64_t global          : 1;
    uint64_t available       : 3;
    uint64_t reserved        : 51;
    uint64_t no_execute      : 1;
  };
  uint64_t bytes;
};

union vaddr {
  struct PACKED {
    uint64_t offset_in_page : 12;
    uint64_t pt_index       : 9;
    uint64_t pd_index       : 9;
    uint64_t pdp_index      : 9;
    uint64_t pml4_index     : 9;
    uint64_t sign_extension : 16;
  };
  addr_t address;
};

#define PT_LENGTH          512
#define PT_SIZE            PAGE_SIZE
#define PT_TMP_START_VADDR 0xffffffffffe00000
#define PT_NUM_LEVELS      4
#define PT_LAST_LEVEL      (PT_NUM_LEVELS - 1)

#define PTEPADDR(pte) ((pte)->bytes & 0x000ffffffffff000)

#define ISFLAGSET(flags, flag)   ((flags & VM_FLAG_##flag) != 0)
#define ISFLAGUNSET(flags, flag) ((flags & VM_FLAG_##flag) == 0)

struct pt {
  union pte entries[PT_LENGTH];
};

static size_t first_free_tmp_index;
static struct pt* kpml4;
static struct pt ALIGNED(PT_SIZE) tmp_pt;

static inline addr_t
get_pml4_paddr(void) {
  addr_t paddr;
  ASM("movq %%cr3, %0" : "=r"(paddr));
  return paddr;
}

static inline void
invalidate_page(addr_t vaddr) {
  ASM("invlpg (%0)" : : "r"(vaddr));
}

static inline void
set_pte(union pte* entry, addr_t addr, enum vm_map_flags flags) {
  entry->bytes = addr & 0x000ffffffffff000;
  entry->present = 1;
  entry->writable = ISFLAGSET(flags, WRITABLE);
  entry->user_accessible = ISFLAGSET(flags, USER);
  entry->no_execute = ISFLAGUNSET(flags, EXECUTABLE);
}

static addr_t
map_page_tmp(addr_t paddr, enum vm_map_flags flags) {
  union vaddr vaddr = {.address = PT_TMP_START_VADDR};
  ASSERT(first_free_tmp_index < PT_LENGTH);
  vaddr.pt_index = first_free_tmp_index & 0x1ff;
  set_pte(&tmp_pt.entries[vaddr.pt_index], paddr, flags);
  do {
    ++first_free_tmp_index;
    if (first_free_tmp_index == vaddr.pdp_index ||
        first_free_tmp_index == vaddr.pd_index)
      ++first_free_tmp_index;
  } while (first_free_tmp_index < PT_LENGTH &&
           tmp_pt.entries[first_free_tmp_index].present);
  return vaddr.address;
}

static void
unmap_page_tmp(addr_t vaddr) {
  union vaddr i, tmp_pt_i;
  ASSERT(vaddr >= PT_TMP_START_VADDR);
  i.address = vaddr;
  tmp_pt_i.address = PT_TMP_START_VADDR;
  ASSERT(i.pt_index != tmp_pt_i.pdp_index && i.pt_index != tmp_pt_i.pd_index);
  tmp_pt.entries[i.pt_index].bytes = 0;
  invalidate_page(vaddr);
  if (i.pt_index < first_free_tmp_index)
    first_free_tmp_index = i.pt_index;
}

void
vm_init(void) {
  size_t *indices, i;
  struct pt* pt;
  union vaddr v = {.address = PT_TMP_START_VADDR};

  /* indices[3] (which is i.pt_index) is unused */
  indices = (size_t[4]){v.pml4_index, v.pdp_index, v.pd_index, v.pt_index};

  kpml4 = (struct pt*)(get_pml4_paddr() + HIGHER_HALF);
  mem_set(&tmp_pt, 0, sizeof(struct pt));

  /* setup mapping path to the temporary page table's entries */
  pt = kpml4;
  for (i = 0; i < PT_LAST_LEVEL; ++i) {
    if (pt->entries[*indices].present && pt != &tmp_pt)
      pt = (struct pt*)(PTEPADDR(&pt->entries[*indices]) + HIGHER_HALF);
    else {
      set_pte(
          &pt->entries[*indices],
          ((addr_t)&tmp_pt) - HIGHER_HALF,
          VM_FLAG_WRITABLE);
      pt = &tmp_pt;
    }
    ++indices;
  }
  /* find the first free index in the temporary page table */
  for (first_free_tmp_index = 0; tmp_pt.entries[first_free_tmp_index].present;
       ++first_free_tmp_index)
    ;

  LOGSUCCESS("virtual memory manager initialized");
}

void
vm_flush_tlb(void) {
  ASM("movq %%cr3, %%rax;"
      "movq %%rax, %%rax"
      :
      :
      : "rax");
}

static addr_t
create_pt(struct pt* pt, size_t index) {
  addr_t vaddr, paddr;
  ASSERT(index < PT_LENGTH);
  ASSERT(pt != NULL);
  paddr = pm_request_page();
  vaddr = map_page_tmp(paddr, VM_FLAG_WRITABLE);
  mem_set((ptr_t)vaddr, 0, PT_SIZE);
  set_pte(&pt->entries[index], paddr, VM_FLAG_WRITABLE);
  return vaddr;
}

/* NOTE: only call this function on PTEs belonging to non leaf PTs */
static void
update_pt_pte_flags(struct pt* pt, size_t index, enum vm_map_flags flags) {
  union pte* entry;
  ASSERT(index < PT_LENGTH);
  ASSERT(pt != NULL);
  entry = &pt->entries[index];
  entry->writable = ISFLAGSET(flags, WRITABLE);
  entry->user_accessible = ISFLAGSET(flags, USER);
  /* NX bit should only be set on leaf PTEs */
  entry->no_execute = 0;
}

static inline size_t
get_pages_per_entry(size_t level) {
  size_t pages = 1;
  for (; level < PT_LAST_LEVEL; ++level)
    pages <<= 9;
  return pages;
}

static inline bool_t
pte_flags_match(union pte* e, enum vm_map_flags flags) {
  bool_t res = TRUE;
  res = res && (e->writable && ISFLAGSET(flags, WRITABLE));
  res = res && (e->user_accessible && ISFLAGSET(flags, USER));
  res = res && (e->no_execute && ISFLAGUNSET(flags, EXECUTABLE));
  return res;
}

static status_t
find_space(
    struct pt* pt,
    size_t* index,
    size_t level,
    addr_t paddr_start,
    size_t pages,
    enum vm_map_flags flags) {
  status_t res;
  bool_t strict;
  addr_t pt_vaddr, paddr;
  size_t pages_per_entry, found_pages, i = *index;
  union pte* e;

  ASSERT(level < PT_NUM_LEVELS);

  strict = (flags & VM_FLAG_STRICT) != 0;
next_free_entry:
  *index = i;
  found_pages = 0;
  for (; found_pages < pages && i < PT_LENGTH && pt->entries[i].present; ++i) {
    if (level == PT_LAST_LEVEL) {
      if (strict)
        return -EOVERLAP;
      else if (pte_flags_match(&pt->entries[i], flags))
        break;
      goto retry_loop;
    }

    pt_vaddr = map_page_tmp(PTEPADDR(&pt->entries[*index]), VM_FLAG_WRITABLE);
    res = find_space(
        (struct pt*)pt_vaddr,
        index + 1,
        level + 1,
        paddr_start,
        pages,
        flags);
    unmap_page_tmp(pt_vaddr);
    if (ISERROR(res)) {
      if (res != -EFULL)
        return res;
      goto retry_loop;
    }

    found_pages += (size_t)res;
    if (res >= (ssize_t)pages)
      return (status_t)found_pages;
    if (index[1] < PT_LENGTH - 1) {
    retry_loop:
      *index = ++i;
      found_pages = 0;
    }
  }

  if (i >= PT_LENGTH)
    return -EFULL;

  pages_per_entry = get_pages_per_entry(level);
  paddr = paddr_start;
  for (; found_pages < pages && i < PT_LENGTH; ++i) {
    e = &pt->entries[i];
    if (e->present && (PTEPADDR(e) != paddr || !pte_flags_match(e, flags)))
      break;
    found_pages += pages_per_entry;
    paddr += PAGE_SIZE;
  }

  if (found_pages < pages) {
    if (strict)
      return -EOVERLAP;
    goto next_free_entry;
  }

  return (status_t)found_pages;
}

static status_t
recurse_map(
    struct pt* pt,
    size_t* index,
    size_t level,
    addr_t* paddr,
    size_t* pages,
    enum vm_map_flags flags) {
  size_t i;
  addr_t pt_vaddr;
  status_t res;

  ASSERT(level < PT_NUM_LEVELS);

  for (i = *index; i < PT_LENGTH && *pages > 0; ++i) {
    if (level == PT_LAST_LEVEL) {
      if (pt->entries[i].present)
        continue;
      set_pte(&pt->entries[i], *paddr, flags);
      *paddr += PAGE_SIZE;
      --(*pages);
      continue;
    }

    if (!pt->entries[i].present)
      pt_vaddr = create_pt(pt, i);
    else
      pt_vaddr = map_page_tmp(PTEPADDR(&pt->entries[i]), VM_FLAG_WRITABLE);
    update_pt_pte_flags(pt, i, flags);
    res = recurse_map(
        (struct pt*)pt_vaddr,
        index + 1,
        level + 1,
        paddr,
        pages,
        flags);
    unmap_page_tmp(pt_vaddr);

    if (ISERROR(res))
      return res;
  }

  return SUCCESS;
}

status_t
vm_map_pages(
    ptr_t table,
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    enum vm_map_flags flags) {
  status_t res;
  size_t indices[4];
  union vaddr vaddr_indices;

  ASSERT(table != NULL);
  ASSERT(vaddr_hint != NULL);
  /* ensure the paddr is page aligned */
  ASSERT(ISPAGEALIGNED(paddr_start));
  ASSERT(ISPAGEALIGNED(*vaddr_hint));

  vaddr_indices.address = *vaddr_hint;

  indices[0] = vaddr_indices.pml4_index;
  indices[1] = vaddr_indices.pdp_index;
  indices[2] = vaddr_indices.pd_index;
  indices[3] = vaddr_indices.pt_index;

  res = find_space(table, indices, 0, paddr_start, pages, flags);
  if (ISERROR(res))
    goto out;

  res = recurse_map(table, indices, 0, &paddr_start, &pages, flags);
  if (ISERROR(res))
    goto out;

  vaddr_indices.pml4_index = indices[0] & 0x1ff;
  vaddr_indices.pdp_index = indices[1] & 0x1ff;
  vaddr_indices.pd_index = indices[2] & 0x1ff;
  vaddr_indices.pt_index = indices[3] & 0x1ff;
  /* sign extend if needed */
  if (indices[0] & 0x100)
    vaddr_indices.address |= (addr_t)0xffff << 48;
  *vaddr_hint = vaddr_indices.address;

out:
  return res;
}

status_t
vm_map_bytes(
    ptr_t table,
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    enum vm_map_flags flags) {
  status_t res;
  addr_t aligned_paddr = PAGEALIGNDOWN(paddr_start);
  bytes = (paddr_start + bytes) - aligned_paddr;
  *vaddr_hint &= ~(size_t)0xfff;
  res = vm_map_pages(table, aligned_paddr, PAGES(bytes), vaddr_hint, flags);
  *vaddr_hint |= paddr_start & 0xfff;
  return res;
}

status_t
vm_kmap_pages(
    addr_t paddr_start,
    size_t pages,
    addr_t* vaddr_hint,
    enum vm_map_flags flags) {
  ASSERT((flags & VM_FLAG_USER) == 0);
  ASSERT(vaddr_hint != NULL);
  if (*vaddr_hint == 0)
    *vaddr_hint = KERNEL_VADDR_END;
  else
    ASSERT(
        *vaddr_hint > KERNEL_VADDR_END ||
        (*vaddr_hint >= KERNEL_HEAP_START && *vaddr_hint < KERNEL_VADDR_START));
  return vm_map_pages(kpml4, paddr_start, pages, vaddr_hint, flags);
}

status_t
vm_kmap_bytes(
    addr_t paddr_start,
    size_t bytes,
    addr_t* vaddr_hint,
    enum vm_map_flags flags) {
  ASSERT((flags & VM_FLAG_USER) == 0);
  ASSERT(vaddr_hint != NULL);
  if (*vaddr_hint == 0)
    *vaddr_hint = KERNEL_VADDR_END;
  else
    ASSERT(
        *vaddr_hint > KERNEL_VADDR_END ||
        (*vaddr_hint >= KERNEL_HEAP_START && *vaddr_hint < KERNEL_VADDR_START));
  return vm_map_bytes(kpml4, paddr_start, bytes, vaddr_hint, flags);
}

static size_t
recurse_unmap(
    struct pt* pt,
    size_t* index,
    size_t level,
    addr_t* vaddr,
    size_t pages) {
  addr_t pt_vaddr;
  size_t unmapped_pages, pages_per_entry = get_pages_per_entry(level);
  for (unmapped_pages = 0; *index < PT_LENGTH && unmapped_pages < pages;
       ++(*index)) {
    union pte* entry = &pt->entries[*index];
    if (level == PT_LAST_LEVEL) {
      entry->bytes = 0;
      invalidate_page(*vaddr);
      ++unmapped_pages;
      *vaddr += PAGE_SIZE;
    } else if (entry->present) {
      pt_vaddr = map_page_tmp(PTEPADDR(entry), VM_FLAG_WRITABLE);
      unmapped_pages += recurse_unmap(
          (struct pt*)pt_vaddr,
          index + 1,
          level + 1,
          vaddr,
          pages);
      unmap_page_tmp(pt_vaddr);
    } else {
      unmapped_pages += pages_per_entry;
      *vaddr += pages_per_entry * PAGE_SIZE;
    }
    if (unmapped_pages >= pages)
      break;
  }
  return unmapped_pages;
}

size_t
vm_unmap_pages(ptr_t table, addr_t vaddr_start, size_t pages) {
  size_t indices[4];
  union vaddr vaddr_indices;

  ASSERT(table != NULL);
  /* ensure the vaddr is aligned */
  ASSERT(ISPAGEALIGNED(vaddr_start));

  vaddr_indices.address = vaddr_start;

  indices[0] = vaddr_indices.pml4_index;
  indices[1] = vaddr_indices.pdp_index;
  indices[2] = vaddr_indices.pd_index;
  indices[3] = vaddr_indices.pt_index;

  return recurse_unmap(table, indices, 0, &vaddr_start, pages);
}

size_t
vm_unmap_bytes(ptr_t table, addr_t vaddr_start, size_t bytes) {
  addr_t aligned_vaddr = PAGEALIGNDOWN(vaddr_start);
  bytes = (vaddr_start + bytes) - aligned_vaddr;
  return vm_unmap_pages(table, aligned_vaddr, PAGES(bytes));
}

size_t
vm_kunmap_pages(addr_t vaddr_start, size_t pages) {
  return vm_unmap_pages(kpml4, vaddr_start, pages);
}

size_t
vm_kunmap_bytes(addr_t vaddr_start, size_t bytes) {
  return vm_unmap_bytes(kpml4, vaddr_start, bytes);
}

static status_t
recurse_find(struct pt* pt, size_t* index, size_t level, addr_t* paddr) {
  union pte* entry;
  addr_t pt_vaddr;
  status_t res;

  ASSERT(level < PT_NUM_LEVELS);

  entry = &pt->entries[*index];
  if (!entry->present)
    return -ENOMAP;
  else if (level == PT_LAST_LEVEL) {
    *paddr = PTEPADDR(entry);
    return SUCCESS;
  }

  pt_vaddr = map_page_tmp(PTEPADDR(entry), VM_FLAG_WRITABLE);
  res = recurse_find((struct pt*)pt_vaddr, index + 1, level + 1, paddr);
  unmap_page_tmp(pt_vaddr);

  return res;
}

status_t
vm_vaddr_to_paddr(ptr_t table, addr_t vaddr, addr_t* paddr) {
  status_t res;
  size_t indices[4];
  union vaddr i = {.address = vaddr};

  ASSERT(paddr != NULL);
  ASSERT(table != NULL);

  indices[0] = i.pml4_index;
  indices[1] = i.pdp_index;
  indices[2] = i.pd_index;
  indices[3] = i.pt_index;

  res = recurse_find(table, indices, 0, paddr);
  if (res == SUCCESS)
    *paddr |= vaddr & 0xfff;

  return res;
}

status_t
vm_kvaddr_to_paddr(addr_t vaddr, addr_t* paddr) {
  return vm_vaddr_to_paddr(kpml4, vaddr, paddr);
}

static status_t
recurse_set_backing(struct pt* pt, size_t* index, size_t level, addr_t paddr) {
  status_t res;
  addr_t pt_vaddr;
  union pte* entry;

  ASSERT(level < PT_NUM_LEVELS);

  entry = &pt->entries[*index];
  if (!entry->present)
    return -ENOMAP;
  if (level == PT_LAST_LEVEL) {
    entry->bytes &= (addr_t)~0x000ffffffffff000;
    entry->bytes |= paddr;
    return SUCCESS;
  }

  pt_vaddr = map_page_tmp(PTEPADDR(entry), VM_FLAG_WRITABLE);
  res = recurse_set_backing((struct pt*)pt_vaddr, index + 1, level + 1, paddr);
  unmap_page_tmp(pt_vaddr);

  return res;
}

status_t
vm_set_backing(ptr_t table, addr_t vaddr, addr_t paddr) {
  size_t indices[4];
  union vaddr vaddr_indices = {.address = vaddr};

  ASSERT(ISPAGEALIGNED(vaddr));
  ASSERT(ISPAGEALIGNED(paddr));

  indices[0] = vaddr_indices.pml4_index;
  indices[1] = vaddr_indices.pdp_index;
  indices[2] = vaddr_indices.pd_index;
  indices[3] = vaddr_indices.pt_index;

  return recurse_set_backing(table, indices, 0, paddr);
}

status_t
vm_kset_backing(addr_t vaddr, addr_t paddr) {
  return vm_set_backing(kpml4, vaddr, paddr);
}

static status_t
recurse_flag(struct pt* pt, size_t* index, size_t level, size_t* pages, enum vm_map_flags flags) {
  size_t i;
  addr_t pt_vaddr;
  status_t res;
  union pte* pte;

  ASSERT(level < PT_NUM_LEVELS);

  for (i = *index; i < PT_LENGTH && *pages > 0; ++i) {
    pte = &pt->entries[i];
    if (level == PT_LAST_LEVEL) {
      if (!pte->present)
        return -ENOMAP;
      set_pte(pte, PTEPADDR(pte), flags);
      --(*pages);
      continue;
    }

    if (!pt->entries[i].present)
      return -ENOMAP;

    pt_vaddr = map_page_tmp(PTEPADDR(&pt->entries[i]), VM_FLAG_WRITABLE);
    update_pt_pte_flags(pt, i, flags);
    res = recurse_flag(
        (struct pt*)pt_vaddr,
        index + 1,
        level + 1,
        pages,
        flags);
    unmap_page_tmp(pt_vaddr);

    if (ISERROR(res))
      return res;
  }

  return SUCCESS;
}

status_t vm_flag_pages(ptr_t table, addr_t vaddr_start, size_t pages, enum vm_map_flags flags) {
  size_t indices[4];
  union vaddr vaddr_indices = {.address = vaddr_start};

  ASSERT(ISPAGEALIGNED(vaddr_start));

  indices[0] = vaddr_indices.pml4_index;
  indices[1] = vaddr_indices.pml4_index;
  indices[2] = vaddr_indices.pml4_index;
  indices[3] = vaddr_indices.pml4_index;

  return recurse_flag(table, indices, 0, &pages, flags);
}

status_t vm_kflag_pages(addr_t vaddr_start, size_t pages, enum vm_map_flags flags) {
  return vm_flag_pages(kpml4, vaddr_start, pages, flags);
}

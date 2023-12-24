#include <mm/vm.h>
#include <mm/pm.h>
#include <mm/page.h>
#include <util/mem.h>
#include <util/attributes.h>
#include <errors.h>
#include <types.h>
#include <assert.h>

union pte {
  struct PACKED {
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user_accessible : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t is_big : 1;
    uint64_t global : 1;
    uint64_t available : 3;
    uint64_t reserved : 51;
    uint64_t no_execute : 1;
  };
  uint64_t bytes;
};

union vaddr {
  struct PACKED {
    uint64_t offset_in_page : 12;
    uint64_t pt_index : 9;
    uint64_t pd_index : 9;
    uint64_t pdp_index : 9;
    uint64_t pml4_index : 9;
    uint64_t sign_extension : 16;
  };
  addr_t address;
};

#define PT_LENGTH 512
#define PT_SIZE PAGE_SIZE
#define PT_TMP_START_VADDR 0xFFFFFFFFFFE00000
#define PT_NUM_LEVELS 4
#define PT_LAST_LEVEL (PT_NUM_LEVELS-1)

#define PTE_PADDR(pte) ((pte)->bytes & 0x000FFFFFFFFFF000)

#define ISFLAGSET(flags, flag) ((flags & VM_MAP_##flag) != 0)
#define ISFLAGUNSET(flags, flag) ((flags & VM_MAP_##flag) == 0)

struct pt {
  union pte entries[PT_LENGTH];
};

static size_t first_free_tmp_index;
static struct pt* pml4;
static struct pt ALIGNED(PT_SIZE) tmp_pt;

static inline addr_t get_pml4_paddr(void) {
  addr_t paddr;
  __asm__("movq %%cr3, %0" : "=r" (paddr));
  return paddr;
}

static inline void invalidate_page(addr_t vaddr) {
  __asm__("invlpg (%0)" : "=r" (vaddr));
}

static inline void set_pte(union pte* entry, addr_t addr, enum vm_map_flags flags) {
  entry->bytes = addr & 0x000FFFFFFFFFF000;
  entry->present = 1;
  entry->writable = ISFLAGSET(flags, WRITABLE);
  entry->user_accessible = ISFLAGSET(flags, USER);
  entry->no_execute = ISFLAGUNSET(flags, EXECUTABLE);
}

static addr_t map_page_tmp(addr_t paddr, enum vm_map_flags flags) {
  union vaddr vaddr = { .address = PT_TMP_START_VADDR };
  ASSERT(first_free_tmp_index < PT_LENGTH);
  vaddr.pt_index = first_free_tmp_index;
  set_pte(&tmp_pt.entries[vaddr.pt_index], paddr, flags);
  do {
    ++first_free_tmp_index;
    if (first_free_tmp_index == vaddr.pdp_index || first_free_tmp_index == vaddr.pd_index)
      ++first_free_tmp_index;
  } while (first_free_tmp_index < PT_LENGTH && tmp_pt.entries[first_free_tmp_index].present);
  return vaddr.address;
}

static void unmap_page_tmp(addr_t vaddr) {
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

void vm_init(void) {
  size_t* indices;
  struct pt* pt;
  union vaddr i = { .address = PT_TMP_START_VADDR };

  indices = (size_t[4]) {
    i.pml4_index,
    i.pdp_index,
    i.pd_index,
    i.pt_index /* unused */
  };

  pml4 = (struct pt*)(get_pml4_paddr() + HIGHER_HALF);
  mem_set(&tmp_pt, 0, sizeof(struct pt));

  /* setup mapping path to the temporary page table's entries */
  pt = pml4;
  for (size_t i = 0; i < PT_LAST_LEVEL; ++i) {
    if (pt->entries[*indices].present && pt != &tmp_pt)
      pt = (struct pt*) (PTE_PADDR(&pt->entries[*indices]) + HIGHER_HALF);
    else {
      set_pte(&pt->entries[*indices], ((addr_t) &tmp_pt) - HIGHER_HALF, VM_MAP_WRITABLE);
      pt = &tmp_pt;
    }
    ++indices;
  }
  /* find the first free index in the temporary page table */
  for (first_free_tmp_index = 0; tmp_pt.entries[first_free_tmp_index].present; ++first_free_tmp_index)
    ;
}

void vm_flush_tlb(void) {
  __asm__("movq %%cr3, %%rax" : : : "rax");
  __asm__("movq %%rax, %%cr3" : : : "rax");
}

static addr_t create_pt(struct pt* pt, size_t index) {
  addr_t vaddr, paddr;
  ASSERT(index < PT_LENGTH);
  ASSERT(pt != NULL);
  paddr = pm_request_page();
  vaddr = map_page_tmp(paddr, VM_MAP_WRITABLE);
  mem_set((ptr_t) vaddr, 0, PT_SIZE);
  set_pte(&pt->entries[index], paddr, VM_MAP_WRITABLE);
  return vaddr;
}

static void update_pt_pte_flags(struct pt* pt, size_t index, enum vm_map_flags flags) {
  union pte* entry;
  ASSERT(index < PT_LENGTH);
  ASSERT(pt != NULL);
  entry = &pt->entries[index];
  entry->writable = ISFLAGSET(flags, WRITABLE);
  entry->user_accessible = ISFLAGSET(flags, USER);
  entry->no_execute = 0;
}

static inline size_t get_pages_per_entry(size_t level) {
  size_t pages = 1;
  for (; level < PT_LAST_LEVEL; ++level)
    pages <<= 9;
  return pages;
}

static status_t find_space(struct pt* pt, size_t* index, size_t level, size_t pages, bool_t strict) {
  addr_t pt_vaddr;
  size_t pages_per_entry, found_pages, i = *index;
  
  ASSERT(level < PT_NUM_LEVELS);

next_free_entry:
  *index = i;
  found_pages = 0;
  for (; found_pages < pages && i < PT_LENGTH && pt->entries[i].present; ++i) {
    status_t res;
    if (level == PT_LAST_LEVEL) {
      if (strict)
        return -EOVERLAP;
      goto retry_loop;
    }

    pt_vaddr = map_page_tmp(PTE_PADDR(&pt->entries[*index]), VM_MAP_WRITABLE);
    res = find_space((struct pt*) pt_vaddr, index + 1, level + 1, pages, strict);
    unmap_page_tmp(pt_vaddr);
    if (ISERROR(res)) {
      if (res != -EFULL)
        return res;
      goto retry_loop;
    }

    found_pages += res;
    if (res >= pages)
      return found_pages;
    if (index[1] < PT_LENGTH-1) {
retry_loop:
      ++i;
      goto next_free_entry;
    }
  }

  if (i >= PT_LENGTH)
    return -EFULL;

  pages_per_entry = get_pages_per_entry(level);
  for (; found_pages < pages && i < PT_LENGTH && !pt->entries[i].present; ++i)
    found_pages += pages_per_entry;

  if (found_pages < pages) {
    if (strict)
      return -EOVERLAP;
    goto next_free_entry;
  }

  return found_pages;
}

static status_t recurse_map(struct pt* pt, size_t* index, size_t level, addr_t* paddr, size_t* pages, enum vm_map_flags flags) {
  ASSERT(level < PT_NUM_LEVELS);

  for (size_t i = *index; i < PT_LENGTH && *pages > 0; ++i) {
    size_t pt_vaddr;
    status_t res;

    if (level == PT_LAST_LEVEL) {
      set_pte(&pt->entries[i], *paddr, flags);
      *paddr += PAGE_SIZE;
      --(*pages);
      continue;
    }

    if (!pt->entries[i].present)
      pt_vaddr = create_pt(pt, i);
    else
      pt_vaddr = map_page_tmp(PTE_PADDR(&pt->entries[i]), VM_MAP_WRITABLE);
    update_pt_pte_flags(pt, i, flags);
    res = recurse_map((struct pt*) pt_vaddr, index + 1, level + 1, paddr, pages, flags);
    unmap_page_tmp(pt_vaddr);

    if (ISERROR(res))
      return res;
  }
  
  return SUCCESS;
}

status_t vm_map_pages(ptr_t table, addr_t paddr_start, size_t pages, addr_t* vaddr_hint, enum vm_map_flags flags) {
  status_t res;
  size_t* indices;
  union vaddr vaddr_indices = {0};

  ASSERT(table != NULL);

  if (flags & VM_MAP_STRICT && vaddr_hint == NULL)
    return -EINVAL;
  if (vaddr_hint != NULL)
    vaddr_indices.address = *vaddr_hint;
  indices = (size_t[4]) {
    vaddr_indices.pml4_index,
    vaddr_indices.pdp_index,
    vaddr_indices.pd_index,
    vaddr_indices.pt_index,
  };

  res = find_space(table, indices, 0, 1, (flags & VM_MAP_STRICT) != 0);
  if (ISERROR(res))
    return res;

  res = recurse_map(table, indices, 0, &paddr_start, &pages, flags);

  if (res == SUCCESS && vaddr_hint != NULL) {
    vaddr_indices.pml4_index = indices[0];
    vaddr_indices.pdp_index = indices[1];
    vaddr_indices.pd_index = indices[2];
    vaddr_indices.pt_index = indices[3];
    *vaddr_hint = vaddr_indices.address;
  }
  
  return res;
}

status_t vm_map_kernel_pages(addr_t paddr_start, size_t pages, addr_t* vaddr_hint, enum vm_map_flags flags) {
  ASSERT((flags & VM_MAP_USER) == 0);
  return vm_map_pages(pml4, paddr_start, pages, vaddr_hint, flags);
}

static size_t recurse_unmap(struct pt* pt, size_t* index, size_t level, size_t pages) {
  addr_t pt_vaddr;
  size_t unmapped_pages;
  for (unmapped_pages = 0; *index < PT_LENGTH; ++(*index)) {
    union pte* entry = &pt->entries[*index];
    if (level == PT_LAST_LEVEL) {
      entry->bytes = 0;
      ++unmapped_pages;
    } else if (entry->present) {
      pt_vaddr = map_page_tmp(PTE_PADDR(entry), VM_MAP_WRITABLE);
      unmapped_pages += recurse_unmap((struct pt*) pt_vaddr, index + 1, level + 1, pages);
      unmap_page_tmp(pt_vaddr);
    } else
      unmapped_pages += get_pages_per_entry(level);
    if (unmapped_pages >= pages)
      break;
  }
  return unmapped_pages;
}

void vm_unmap_pages(ptr_t table, addr_t vaddr_start, size_t pages) {
  size_t* indices;
  union vaddr vaddr_indices = { .address = vaddr_start };
  
  ASSERT(table != NULL);
  
  indices = (size_t[4]) {
    vaddr_indices.pml4_index,
    vaddr_indices.pdp_index,
    vaddr_indices.pd_index,
    vaddr_indices.pt_index
  };
  
  recurse_unmap(table, indices, 0, pages);
  for (; pages > 0; --pages, vaddr_start += PAGE_SIZE)
    invalidate_page(vaddr_start);
}

void vm_unmap_kernel_pages(addr_t vaddr_start, size_t pages) {
  vm_unmap_pages(pml4, vaddr_start, pages);
}

static status_t recurse_find(struct pt* pt, size_t* index, size_t level, addr_t* paddr) {
  union pte* entry;
  addr_t pt_vaddr;
  status_t res;

  ASSERT(level < PT_NUM_LEVELS);

  entry = &pt->entries[*index];
  if (!entry->present)
    return -ENOMAP;
  else if (level == PT_LAST_LEVEL) {
    *paddr = PTE_PADDR(entry);
    return SUCCESS;
  }

  pt_vaddr = map_page_tmp(PTE_PADDR(entry), VM_MAP_WRITABLE);
  res = recurse_find((struct pt*) pt_vaddr, index + 1, level + 1, paddr);
  unmap_page_tmp(pt_vaddr);

  return res;
}

status_t vm_vaddr_to_paddr(ptr_t table, addr_t vaddr, addr_t* paddr) {
  size_t* indices;
  union vaddr i = { .address = vaddr };

  ASSERT(paddr != NULL);
  ASSERT(table != NULL);

  indices = (size_t[4]) {
    i.pml4_index,
    i.pdp_index,
    i.pd_index,
    i.pt_index
  };

  return recurse_find(table, indices, 0, paddr);
}

status_t vm_kernel_vaddr_to_paddr(addr_t vaddr, addr_t* paddr) {
  return vm_vaddr_to_paddr(pml4, vaddr, paddr);
}

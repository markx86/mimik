#include <mm/pm.h>
#include <mm/vm.h>
#include <mem/mem.h>
#include <mem/layout.h>
#include <structs/list.h>
#include <kernel.h>
#include <assert.h>

struct mem_info {
  size_t total;
  size_t used;
  size_t free;
};

struct free_page {
  uint8_t unused[PAGE_SIZE - sizeof(struct list)];
  struct list link;
};

static addr_t page_pointer;
static struct list freelist;
static struct mem_info mem_info;
static struct bootinfo_mem_map* mem_map;

static inline void
ensure_mem_map_is_sorted(void) {
  size_t i, j;
  struct bootinfo_mem_segment *seg, *next_seg, *xchg_seg, tmp_seg;

  ASSERT(!K.inited.pm);

  /* bubble sort the memory map */
  for (i = 0; i < mem_map->entries - 1; ++i) {
    seg = mem_map->segments + i;
    xchg_seg = NULL;
    /* check if this segment is out of order segment */
    for (j = i + 1; j < mem_map->entries; ++j) {
      next_seg = mem_map->segments + j;
      if (seg->addr > next_seg->addr)
        xchg_seg = next_seg;
    }
    /* exchange segments if needed */
    if (xchg_seg) {
      tmp_seg = *xchg_seg;
      *xchg_seg = *seg;
      *seg = tmp_seg;
    }
  }
}

static inline void
parse_mem_map(void) {
  struct bootinfo_mem_segment* seg;
  size_t i;
  addr_t seg_start, seg_end;

  ASSERT(!K.inited.pm);
  mem_set(&mem_info, 0, sizeof(mem_info));

  for (i = 0; i < mem_map->entries; ++i) {
    seg = mem_map->segments + i;
    seg_start = seg->addr;
    seg_end = seg_start + seg->length;
    mem_info.total += seg->length;
    switch (seg->type) {
      case BOOTINFO_MEM_SEGMENT_TYPE_AVAILABLE:
        if (page_pointer == 0)
          page_pointer = seg->addr;
        mem_info.free += seg->length;
        break;
      case BOOTINFO_MEM_SEGMENT_TYPE_RESERVED:
        if (page_pointer >= seg_start && page_pointer < seg_end)
          page_pointer = 0;
        mem_info.used += seg->length;
        break;
      default:
        UNREACHABLE();
    }
  }
}

static inline void
ensure_page_pointer_is_valid(void) {
  size_t i;
  struct bootinfo_mem_segment* seg;
  addr_t seg_start, seg_end;

  ASSERT(K.inited.pm);

  for (i = 0; i < mem_map->entries; ++i) {
    seg = mem_map->segments + i;
    seg_start = seg->addr;
    seg_end = seg_start + seg->length;
    if (seg->type == BOOTINFO_MEM_SEGMENT_TYPE_AVAILABLE) {
      if (page_pointer == 0)
        page_pointer = seg->addr;
      continue;
    }
    if (page_pointer >= seg_start && page_pointer < seg_end)
      page_pointer = 0;
  }
}

static inline addr_t
next_untracked_page(void) {
  addr_t prev_page_pointer;

  ASSERT(K.inited.pm);

  prev_page_pointer = page_pointer;
  page_pointer += PAGE_SIZE;

  ensure_page_pointer_is_valid();

  ASSERT(prev_page_pointer < page_pointer);
  /* FIXME: out of memory condition, handle this better */
  ASSERT(page_pointer != 0);

  return prev_page_pointer;
}

void
pm_init(void) {
  addr_t first_free_page;

  ASSERT(!K.inited.pm);

  mem_map = &K.bootinfo->mem_map;
  page_pointer = first_free_page = PAGEALIGNUP(K.bootinfo->first_free_paddr);
  list_init(&freelist);

  ensure_mem_map_is_sorted();
  parse_mem_map();

  ASSERT(page_pointer >= first_free_page);

  LOGSUCCESS("physical memory manager initialized");
  K.inited.pm = TRUE;
}

// static bool_t
// is_reserved(addr_t paddr) {
//   size_t i;
//   addr_t seg_start, seg_end;
//   struct bootinfo_mem_segment* seg;
//   for (i = 0; i < mem_map->entries; ++i) {
//     seg = mem_map->segments + i;
//     seg_start = seg->addr;
//     seg_end = seg_start + seg->length;
//     if (UNLIKELY(paddr >= seg_start && paddr < seg_end)) {
//       if (seg->type == BOOTINFO_MEM_SEGMENT_TYPE_RESERVED)
//         return TRUE;
//     }
//   }
//   return FALSE;
// }

// void pm_track_page(addr_t paddr) {
//   struct free_page* page = (struct free_page*)layout_paddr_to_vaddr(paddr);
//   if (!is_reserved(paddr))
//     mem_set(page, 0, sizeof(*page));
//   list_insert(&freelist, &page->link);
// }

addr_t
pm_request_page(void) {
  struct free_page* page;
  addr_t page_paddr;

  ASSERT(K.inited.pm);

  if (UNLIKELY(list_is_empty(&freelist)))
    page_paddr = next_untracked_page();
  else {
    ASSERT(&freelist != freelist.next);
    page = CONTAINEROF(freelist.next, struct free_page, link);
    list_remove(&page->link);
    mem_set(page, 0, sizeof(*page));
    page_paddr = layout_vaddr_to_paddr((addr_t)page);
  }

  return page_paddr;
}

void
pm_lock_pages(addr_t paddr, size_t n) {
  struct free_page* page;

  ASSERT(K.inited.pm);

  if (paddr > page_pointer) {
    page_pointer = paddr + PAGE_SIZE;
    ensure_page_pointer_is_valid();
  } else {
    list_for_each(page, &freelist, link) {
      if (UNLIKELY(paddr == layout_vaddr_to_paddr((addr_t)page))) {
        list_remove(&page->link);
        --n;
      }
      if (n == 0)
        break;
    }
  }
}

void
pm_release_pages(addr_t paddr, size_t n) {
  struct free_page* page = (struct free_page*)layout_paddr_to_vaddr(paddr);
  ASSERT(K.inited.pm);
  ASSERT(paddr + BYTES(n) < page_pointer);
  for (; n > 0; --n) {
    list_insert(&freelist, &page->link);
    ++page;
  }
}

size_t
pm_get_total_memory(void) {
  return mem_info.total;
}

#include <assert.h>
#include <errors.h>
#include <kernel.h>
#include <mm/page.h>
#include <mm/pm.h>
#include <mm/vm.h>
#include <structs/bitmap.h>
#include <structs/list.h>
#include <util/mem.h>

#define BITMAP_BYTES PAGE_SIZE
#define BITMAP_SIZE (BITMAP_BYTES << 3)
#define BITMAP_PAGELOCKED TRUE
#define BITMAP_PAGEFREE FALSE

struct mem_info {
  size_t used;
  size_t free;
};

struct mem_bitmap {
  struct list link;
  size_t index;
  size_t first_free;
  struct bitmap super;
};

static struct mem_info mem_info;
static struct list mem_bitmaps;
static struct mem_bitmap* first_free_bitmap;
static addr_t next_bitmap_page;
/* Initial bitmap that tracks the first 128MB of memory */
static struct mem_bitmap mem_bitmap0;
static char mem_bitmap0_page[BITMAP_BYTES];

#define TOBITMP(a) ((a) >> 27)
#define TOINDEX(a) (((a) & 0x7FFF000) / PAGE_SIZE)
#define TOPADDR(b, i) (((b) << 27) + (i) * PAGE_SIZE)

static void
compute_mem_size(struct bootinfo_mem_map* mem_map) {
  mem_set(&mem_info, 0, sizeof(struct mem_info));
  for (size_t i = 0; i < mem_map->entries; ++i) {
    struct bootinfo_mem_segment* segment = &mem_map->segments[i];
    mem_info.free += segment->length;
    if (segment->type == BOOTINFO_MEM_SEGMENT_TYPE_RESERVED &&
        TOBITMP(segment->addr) == 0)
      pm_try_lock_pages(segment->addr, segment->length);
  }
}

static void
init_first_bitmap(void) {
  list_init(&mem_bitmaps);
  mem_bitmap0.index = 0;
  mem_bitmap0.first_free = 0;
  mem_bitmap0.super = bitmap_from(BITMAP_SIZE, mem_bitmap0_page);
  list_insert(&mem_bitmaps, &mem_bitmap0.link);
  first_free_bitmap = &mem_bitmap0;
}

void
pm_init(addr_t free_mem_ptr) {
  init_first_bitmap();
  compute_mem_size(&kcfg.bootinfo->mem_map);
  pm_try_lock_pages_range(KERNEL_START_PADDR, KERNEL_END_PADDR);
  next_bitmap_page = pm_request_page();
}

addr_t
pm_request_page(void) {
  addr_t paddr;
  paddr = TOPADDR(first_free_bitmap->index, first_free_bitmap->first_free++);
  pm_try_lock_page(paddr);
  return paddr;
}

addr_t
pm_request_pages(size_t num) {
  struct mem_bitmap *mem_bitmap, *next;
  addr_t paddr;
  status_t err;
  size_t start_index, index, found = 0;
  mem_bitmap = first_free_bitmap;
  for (index = mem_bitmap->first_free, start_index = index;
       found < num && index < mem_bitmap->super.size;
       ++index) {
  retry:
    err = bitmap_get(&mem_bitmap->super, index);
    if (ISERROR(err)) {
      ASSERT(err == -EINVAL);
      next = containerof(mem_bitmap->link.next, struct mem_bitmap, link);
      if (next->index - mem_bitmap->index > 1) {
        found = 0;
        index = next->index * BITMAP_SIZE;
        start_index = index;
      }
      mem_bitmap = next;
      goto retry;
    }
    if (err == BITMAP_PAGEFREE)
      ++found;
    else {
      start_index = index + 1;
      found = 0;
    }
  }
  paddr = TOPADDR(mem_bitmap->index, start_index);
  pm_try_lock_pages(paddr, BYTES(num));
  return paddr;
}

addr_t
pm_request_bytes(size_t bytes) {
  return pm_request_pages(PAGEALIGN_UP(bytes) / PAGE_SIZE);
}

static struct mem_bitmap*
create_bitmap(size_t index) {
  struct mem_bitmap *mem_bitmap, *new_bitmap;
  list_for_each(mem_bitmap, &mem_bitmaps, link) {
    if (mem_bitmap->index > index)
      break;
  }
  new_bitmap = NULL; /* TODO: allocate new bitmap */
  ASSERT(new_bitmap != NULL);
  new_bitmap->first_free = 0;
  new_bitmap->index = 0;
  new_bitmap->super = bitmap_from(BITMAP_SIZE, (ptr_t)next_bitmap_page);
  list_insert(mem_bitmap->link.prev, &new_bitmap->link);
  next_bitmap_page = pm_request_page();
  return new_bitmap;
}

static void
find_first_free_bitmap(void) {
  struct mem_bitmap *mem_bitmap, *prev_bitmap = NULL;
  list_for_each(mem_bitmap, &first_free_bitmap->link, link) {
    if (mem_bitmap->super.unset > 0) {
      first_free_bitmap = mem_bitmap;
      return;
    } else if (
        prev_bitmap == NULL || mem_bitmap->index - prev_bitmap->index == 1)
      prev_bitmap = mem_bitmap;
  }
  ASSERT(prev_bitmap != NULL);
  first_free_bitmap = create_bitmap(prev_bitmap->index + 1);
}

static struct mem_bitmap*
find_bitmap_by_index(size_t index) {
  struct mem_bitmap* mem_bitmap;
  list_for_each(mem_bitmap, &mem_bitmaps, link) {
    if (mem_bitmap->index == index)
      return mem_bitmap;
  }
  return NULL;
}

static bool_t
find_first_free_index(void) {
  do {
    status_t err =
        bitmap_get(&first_free_bitmap->super, first_free_bitmap->first_free);
    if (ISERROR(err)) {
      ASSERT(err == -EINVAL);
      find_first_free_bitmap();
    } else if (err == BITMAP_PAGEFREE)
      return TRUE;
  } while (++first_free_bitmap->first_free < first_free_bitmap->super.size);
  return FALSE;
}

static bool_t
try_set_page(addr_t paddr, bool_t state) {
  struct mem_bitmap* mem_bitmap;
  status_t sts;
  size_t bitmap, index;
  bitmap = TOBITMP(paddr);
  mem_bitmap = find_bitmap_by_index(bitmap);
  if (mem_bitmap == NULL)
    mem_bitmap = create_bitmap(bitmap);
  index = TOINDEX(paddr);
  sts = bitmap_set(&mem_bitmap->super, index, state);
  return sts == SUCCESS;
}

bool_t
pm_try_lock_page(addr_t paddr) {
  if (try_set_page(paddr, BITMAP_PAGELOCKED)) {
    size_t bitmap = TOBITMP(paddr);
    if (first_free_bitmap->index == bitmap && !find_first_free_index())
      find_first_free_bitmap();
    mem_info.used += PAGE_SIZE;
    mem_info.free -= PAGE_SIZE;
    return TRUE;
  }
  return FALSE;
}

bool_t
pm_try_lock_pages(addr_t paddr, size_t bytes) {
  size_t pages;
  bool_t no_collisions;
  if (bytes == 0)
    return pm_try_lock_page(paddr);
  pages = PAGES(bytes);
  no_collisions = TRUE;
  for (size_t page = 0; page < pages; ++page) {
    no_collisions &= pm_try_lock_page(paddr);
    paddr += PAGE_SIZE;
  }
  return no_collisions;
}

bool_t
pm_try_lock_pages_range(addr_t paddr_start, addr_t paddr_end) {
  return pm_try_lock_pages(paddr_start, paddr_end - paddr_start);
}

bool_t
pm_try_release_page(addr_t paddr) {
  if (try_set_page(paddr, BITMAP_PAGEFREE)) {
    size_t index, bitmap = TOBITMP(paddr);
    if (bitmap < first_free_bitmap->index)
      first_free_bitmap = find_bitmap_by_index(bitmap);
    if (bitmap == first_free_bitmap->index &&
        (index = TOINDEX(paddr)) < first_free_bitmap->first_free)
      first_free_bitmap->first_free = index;
    mem_info.used -= PAGE_SIZE;
    mem_info.free += PAGE_SIZE;
    return TRUE;
  }
  return FALSE;
}

bool_t
pm_try_release_pages(addr_t paddr, size_t bytes) {
  size_t pages;
  bool_t no_collisions;
  if (bytes == 0)
    return pm_try_release_page(paddr);
  pages = PAGES(bytes);
  no_collisions = TRUE;
  for (size_t page = 0; page < pages; ++page) {
    no_collisions |= pm_try_release_page(paddr);
    paddr += PAGE_SIZE;
  }
  return no_collisions;
}

bool_t
pm_try_release_pages_range(addr_t paddr_start, addr_t paddr_end) {
  return pm_try_release_pages(paddr_start, paddr_end - paddr_start);
}

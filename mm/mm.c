#include <mm/mm.h>
#include <mm/pm.h>
#include <mem/mem.h>
#include <mem/layout.h>
#include <log/log.h>
#include <structs/list.h>
#include <assert.h>
#include <kernel.h>

#define SLAB_GROW_SIZE PAGE_SIZE

enum slab_type {
  SLAB_16 = 0,
  SLAB_24,
  SLAB_32,
  SLAB_48,
  SLAB_64,
  SLAB_96,
  SLAB_128,
  SLAB_192,
  SLAB_256,
  SLAB_384,
  SLAB_512,
  SLAB_768,
  SLAB_1024,
  SLAB_2048,
  SLAB_3072,
  SLAB_4096,
  SLAB_MAX
};

STATICASSERT(SLAB_MAX <= 0x10, "too many slab types!");

struct slab {
  enum slab_type type;
  struct list freelist;
  addr_t start;
  addr_t end;
};

#define SLABTYPE(addr) (((addr) >> 36) & 0xffff)
#define SLABBASE(type) (LAYOUT_HEAP_START + ((addr_t)(type) << 36))

static struct slab slabs[SLAB_MAX];

void
mm_init(void) {
  size_t i;
  struct slab* s;
  ASSERT(!K.inited.mm);
  ASSERT(K.inited.pm);
  ASSERT(K.inited.vm);
  for (i = 0; i < SLAB_MAX; ++i) {
    s = slabs + i;
    s->type = (enum slab_type)i;
    s->end = s->start = SLABBASE(s->type);
    list_init(&s->freelist);
  }
  LOGSUCCESS("kernel heap initialized");
  K.inited.mm = TRUE;
}

static inline size_t
ceil_to_power_of_2(size_t sz) {
  --sz;
  sz |= sz >> 1;
  sz |= sz >> 2;
  sz |= sz >> 4;
  sz |= sz >> 8;
  sz |= sz >> 16;
  sz |= sz >> 32;
  return ++sz;
}

/* FIXME: this shit is ugly af, can't we just do a look-up table, please? */
static inline size_t
slab_size(enum slab_type type) {
  size_t sz, shift = ((type & 0xe) >> 1) + 4;
  sz = 1 << shift--;
  if (type & 1)
    sz += 1 << shift;
  if (UNLIKELY(sz > 1024)) {
    sz += 512;
    if (UNLIKELY(sz > 2048))
      sz += 512;
  }
  return sz;
}

static inline struct slab*
get_slab_from_size(size_t sz) {
  enum slab_type type;
  size_t chunk_sz = ceil_to_power_of_2(sz);
  chunk_sz >>= 4;
  for (type = 0; chunk_sz > 0; ++type)
    chunk_sz >>= 1;
  /* check if the object can fit in a smaller bin
     (eg. select 64 bytes bin by rounding up,
     but the object can fit in 48 bytes bin) */
  if (slab_size(type - 1) >= sz)
    --type;
  ASSERT(type < SLAB_MAX);
  return slabs + type;
}

static void
alloc_slab(struct slab* s) {
  struct list* chunk;
  size_t chunk_size;
  addr_t p, next;
  status_t res;

  ASSERT(s->type < SLAB_MAX);

  p = PAGEALIGNUP(s->end);
  res = mm_map(p, SLAB_GROW_SIZE, VM_FLAG_WRITE);
  ASSERT(res == SUCCESS);

  chunk_size = slab_size(s->type);
  p += SLAB_GROW_SIZE;
  for (next = s->end + chunk_size; next < p; next += chunk_size) {
    chunk = (struct list*)s->end;
    list_insert(&s->freelist, chunk);
    s->end = next;
  }
}

ptr_t
mm_alloc(size_t sz) {
  struct slab* slab;
  struct list* chunk;
  ASSERT(sz < slab_size(SLAB_MAX - 1));
  slab = get_slab_from_size(sz);
  if (list_is_empty(&slab->freelist))
    alloc_slab(slab);
  chunk = slab->freelist.next;
  list_remove(chunk);
  return chunk;
}

static inline struct slab*
get_slab_from_address(addr_t addr) {
  struct slab* s;
  size_t sz;
  enum slab_type type = SLABTYPE(addr);
  ASSERT(type < SLAB_MAX);
  s = slabs + type;
  sz = slab_size(type);
  /* verify that the address lies within the slab bounds */
  ASSERT(addr >= s->start && addr + sz < s->end);
  /* ensure the address is aligned to the slab object size */
  ASSERT((addr & (sz - 1)) == 0);
  return s;
}

void
mm_free(ptr_t* alloc) {
  struct slab* slab = get_slab_from_address((addr_t)*alloc);
  mem_set(*alloc, 0, slab_size(slab->type)); /* clear chunk data */
  list_insert(&slab->freelist, (struct list*)*alloc);
  *alloc = NULL;
}

status_t
mm_map(addr_t vaddr, size_t size, flags_t flags) {
  size_t pages;
  status_t res;
  addr_t cur_paddr, cur_vaddr;

  pages = PAGES(size);
  res = vmk_reserve_pages(vaddr, pages, flags);
  if (ISERROR(res))
    return res;

  for (cur_vaddr = vaddr; pages > 0; --pages) {
    cur_paddr = pm_request_page();
    /* TODO: add exception handling for pm_request_page() */
    res = vmk_map_edit_paddr(cur_vaddr, cur_paddr);
    ASSERT(res == SUCCESS);
    cur_vaddr += PAGE_SIZE;
  }

  return SUCCESS;
}

status_t
mm_map_in(struct pt* table, addr_t vaddr, size_t size, flags_t flags) {
  size_t pages;
  status_t res;
  addr_t cur_paddr, cur_vaddr;

  pages = PAGES(size);
  res = vm_reserve_pages(table, vaddr, pages, flags);
  if (ISERROR(res))
    return res;

  for (cur_vaddr = vaddr; pages > 0; --pages) {
    cur_paddr = pm_request_page();
    /* TODO: add exception handling for pm_request_page() */
    res = vm_map_edit_paddr(table, cur_vaddr, cur_paddr);
    ASSERT(res == SUCCESS);
    cur_vaddr += PAGE_SIZE;
  }

  return SUCCESS;
}

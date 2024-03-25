#include <mm/mm.h>
#include <mm/pm.h>
#include <mm/vm.h>
#include <util/align.h>
#include <util/compiler.h>
#include <log/log.h>
#include <assert.h>

#define HEAP_START 0xffffff8000000000

#define HEADER_MAGIC0 'H'
#define HEADER_MAGIC1 'D'
#define HEADER_MAGIC2 'R'

#define MIN_GROW_PAGES 8
#define MIN_BLOCK_SIZE 64

#define ISHEADER(x)                                                    \
  ((x)->magic[0] == HEADER_MAGIC0 && (x)->magic[1] == HEADER_MAGIC1 && \
   (x)->magic[2] == HEADER_MAGIC2)
#define NEXTBLOCK(x) ((struct header*)((addr_t)(x) + (x)->size))

struct PACKED header {
  char magic[3];
  bool_t free;
  uint32_t size;
  struct header* prev;
};

static addr_t heap_end, heap_start = HEAP_START;
static struct header *first_free, *last_block;

void
mm_init(void) {
  status_t res;
  addr_t paddr = pm_request_pages(MIN_GROW_PAGES);
  res = vm_kmap_pages(paddr, MIN_GROW_PAGES, &heap_start, VM_MAP_WRITABLE);
  ASSERT(res == SUCCESS);
  first_free = (struct header*)heap_start;
  first_free->magic[0] = HEADER_MAGIC0;
  first_free->magic[1] = HEADER_MAGIC1;
  first_free->magic[2] = HEADER_MAGIC2;
  first_free->free = TRUE;
  first_free->size = BYTES(MIN_GROW_PAGES);
  first_free->prev = NULL;
  last_block = first_free;
  heap_end = heap_start + first_free->size;
  LOGSUCCESS("kernel heap initialized");
}

static void
grow_heap(size_t sz) {
  status_t res;
  addr_t paddr;
  size_t bytes, pages = PAGES(sz);
  pages = pages >= MIN_GROW_PAGES ? pages : MIN_GROW_PAGES;
  /* verify that the end of the last block in the heap,
           aligns with the end of the heap */
  ASSERT((addr_t)NEXTBLOCK(last_block) == heap_end);
  /* map more memory */
  paddr = pm_request_pages(pages);
  res = vm_kmap_pages(paddr, pages, &heap_end, VM_MAP_WRITABLE | VM_MAP_STRICT);
  ASSERT(res == SUCCESS); /* TODO: handle out of virtual memory exception */
  heap_end += BYTES(pages);
  /* if the last_block is not free, append a new block */
  if (!last_block->free) {
    struct header* new_block = NEXTBLOCK(last_block);
    new_block->magic[0] = HEADER_MAGIC0;
    new_block->magic[1] = HEADER_MAGIC1;
    new_block->magic[2] = HEADER_MAGIC2;
    new_block->free = TRUE;
    new_block->size = 0; /* NOTE: the size is set after the if block */
    new_block->prev = last_block;
    last_block = new_block;
  }
  /* update the last block's size */
  bytes = BYTES(pages);
  ASSERT(bytes < UINT32_MAX);
  last_block->size += (uint32_t)bytes;
}

static struct header*
find_worst_fit(size_t size) {
  struct header *best_hdr, *hdr = first_free;
  best_hdr = NULL;
  /* loop through every block */
  while (hdr <= last_block) {
    ASSERT(ISHEADER(hdr));
    /* ensure the block is free and has enough size */
    if (!hdr->free || hdr->size < size)
      goto next_block;
    /* check if this block is better than the best block size */
    if (best_hdr == NULL || best_hdr->size < hdr->size)
      best_hdr = hdr;
  next_block:
    hdr = NEXTBLOCK(hdr);
  }
  /* return the best block or grow the heap if none was found */
  if (best_hdr != NULL)
    return best_hdr;
  grow_heap(size);
  ASSERT(last_block->free);
  return last_block;
}

static void
split_free_block_at_size(struct header* block, size_t sz) {
  struct header* next;
  /* verify the block is free */
  ASSERT(block->free);
  /* verify the size of the new block is at least the minimum allowed */
  ASSERT(sz >= MIN_BLOCK_SIZE);

  next = (struct header*)((addr_t)block + sz);
  /* check that the next block will fit on the heap */
  if ((addr_t)next + MIN_BLOCK_SIZE > heap_end)
    grow_heap(0); /* 0 == grow the heap by MIN_GROW_PAGES */

  /* update first free and last block */
  if (block == first_free)
    first_free = next;
  if (block == last_block)
    last_block = next;

  /* initialize the new block */
  next->free = TRUE;
  next->size = (uint32_t)(block->size - sz);
  ASSERT(next->size >= MIN_BLOCK_SIZE);
  next->magic[0] = HEADER_MAGIC0;
  next->magic[1] = HEADER_MAGIC1;
  next->magic[2] = HEADER_MAGIC2;
  next->prev = block;

  /* update the block size */
  block->size = (uint32_t)sz;
}

ptr_t
mm_alloc(size_t sz) {
  struct header* hdr;
  ASSERT(sz <= UINT32_MAX);
  /* the size of the block is the size of the allocation
     plus the size of the block header */
  sz += sizeof(struct header);
  /* make sure the block is at least the minimum size */
  sz = sz >= MIN_BLOCK_SIZE ? sz : MIN_BLOCK_SIZE;
  hdr = find_worst_fit(sz);
  /* truncate the selected block to size */
  split_free_block_at_size(hdr, sz);
  hdr->free = FALSE;
  return hdr + 1;
}

static struct header*
find_most_aligned(size_t sz, size_t al) {
  size_t best_offset;
  struct header *best_hdr, *hdr = first_free;
  best_hdr = NULL;
  best_offset = al;
  /* loop through every block */
  while (hdr <= last_block) {
    size_t offset, needed_size;
    /* verify the header is valid */
    ASSERT(ISHEADER(hdr));
    /* ensure the header is free */
    if (!hdr->free)
      goto next_block;
    /* compute the offset of the aligned pointer from the end of the header */
    offset = (addr_t)(hdr + 1);
    offset = ALIGNUP(offset, al) - offset;
    /* compute the block size needed */
    needed_size = sz + offset;
    if (offset == 0)
      needed_size += sizeof(struct header*);
    needed_size = needed_size >= MIN_BLOCK_SIZE ? needed_size : MIN_BLOCK_SIZE;
    /* ensure there's enough space in the block */
    if (hdr->size < needed_size)
      goto next_block;
    /* check if there's enough extra space for the pointer to the header */
    if (offset != 0 && offset < sizeof(struct header*))
      goto next_block;
    /* check if this block is better than the best one found */
    if (best_hdr != NULL &&
        (best_offset < offset ||
         (best_offset == offset && best_hdr->size > hdr->size)))
      goto next_block;
    best_hdr = hdr;
    best_offset = offset;
  next_block:
    hdr = NEXTBLOCK(hdr);
  }
  /* return the best block found or grow the heap and retry */
  if (best_hdr != NULL)
    return best_hdr;
  grow_heap(sz);
  ASSERT(last_block->free);
  return last_block;
}

ptr_t
mm_aligned_alloc(size_t sz, size_t al) {
  ptr_t ptr;
  size_t actual_size;
  struct header *hdr, **hdr_ptr;
  ASSERT(sz <= UINT32_MAX);
  /* find the header with the least amount of space wasted on alignment */
  hdr = find_most_aligned(sz, al);
  /* compute the data pointer */
  ptr = (ptr_t)ALIGNUP((addr_t)(hdr + 1), al);
  /* if needed, insert pointer to the header before the data pointer */
  if ((addr_t)ptr > (addr_t)(hdr + 1)) {
    hdr_ptr = (struct header**)ptr - 1;
    *hdr_ptr = hdr;
  }
  /* compute the actual block size used */
  actual_size = ((addr_t)ptr + sz) - (addr_t)hdr;
  /* truncate the block to actual_size */
  split_free_block_at_size(hdr, actual_size);
  hdr->free = FALSE;
  return ptr;
}

static void
join_free_backwards(struct header* this) {
  struct header* next;
  ASSERT(ISHEADER(this));
  /* nothing to join if this header is not free */
  if (!this->free)
    return;
  /* can't join backwards if there's no previous block
     or if it is not free */
  if (this->prev == NULL || !this->prev->free)
    return;
  this->prev->size += this->size;
  /* if this was the last block,
     make the newly formed block the last one */
  if (this == last_block) {
    last_block = this->prev;
    return;
  }
  /* if it is not update the prev pointer for the next block */
  next = NEXTBLOCK(this);
  ASSERT(ISHEADER(next));
  next->prev = this->prev;
}

void
mm_free(ptr_t alloc) {
  struct header* hdr = (struct header*)alloc - 1;
  if (!ISHEADER(hdr)) {
    /* assume the allocation is aligned */
    hdr = *((struct header**)alloc - 1);
    /* verify the header address is valid */
    ASSERT((addr_t)hdr >= heap_start);
    ASSERT((addr_t)(hdr + 1) < heap_end);
  }
  ASSERT(ISHEADER(hdr));
  /* flag this block as free */
  hdr->free = TRUE;
  if (hdr != last_block) {
    /* update first_free if needed */
    if (first_free > hdr)
      first_free = hdr;
    /* try joining the block with the next one */
    join_free_backwards(NEXTBLOCK(hdr));
  }
  /* try joining the block with the previous one */
  join_free_backwards(hdr);
}

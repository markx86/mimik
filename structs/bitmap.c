#include <errno.h>
#include <structs/bitmap.h>
#include <util/mem.h>

struct bitmap bitmap_from(size_t sz, void* ptr) {
  size_t size = sz, bytes = (size + 7) >> 3;
  mem_set(ptr, 0, bytes);
  return (struct bitmap){
      .bytes = bytes,
      .size = size,
      .unset = size,
      .map = ptr,
  };
}

status_t bitmap_get(struct bitmap* b, size_t i) {
  size_t byte;
  uint8_t bit;
  if (i >= b->size)
    return -EINVAL;
  byte = i >> 3;
  bit = i & 0b111;
  return (b->map[byte] >> bit) & 1;
}

status_t bitmap_set(struct bitmap* b, size_t i, bool_t v) {
  size_t byte;
  uint8_t bit, mask;
  if (i >= b->size)
    return -EINVAL;
  byte = i >> 3;
  bit = i & 0b111;
  if (((b->map[byte] >> bit) & 1) == v)
    return SNOCHANGE;
  mask = 1 << bit;
  if (v) {
    b->map[byte] |= mask;
    --b->unset;
  } else {
    b->map[byte] &= ~mask;
    ++b->unset;
  }
  return SUCCESS;
}

#ifndef MIMIK_BITMAP_H
#define MIMIK_BITMAP_H

#include <types.h>

#define SNOCHANGE 10

struct bitmap {
  size_t bytes;
  size_t size;
  size_t unset;
  uint8_t* map;
};

struct bitmap bitmap_from(size_t sz, void* addr);
status_t bitmap_get(struct bitmap* b, size_t i);
status_t bitmap_set(struct bitmap* b, size_t i, bool_t v);

#endif

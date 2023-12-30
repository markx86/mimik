#ifndef MIMIK_MM_H
#define MIMIK_MM_H

#include <types.h>

void mm_init(void);
void* mm_alloc(size_t sz);
void* mm_aligned_alloc(size_t sz, size_t al);
void mm_free(void* alloc);

#endif

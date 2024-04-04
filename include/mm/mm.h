#ifndef MIMIK_MM_H
#define MIMIK_MM_H

#include <types.h>

void mm_init(void);
ptr_t mm_alloc(size_t sz);
ptr_t mm_aligned_alloc(size_t sz, size_t al);
void mm_free(ptr_t* alloc);

#endif

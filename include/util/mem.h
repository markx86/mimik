#ifndef MIMIK_MEM_H
#define MIMIK_MEM_H

#include <types.h>

void mem_set(ptr_t p, uint8_t c, size_t s);
void mem_copy(ptr_t dst, ptr_t src, size_t sz);
void mem_copy_reverse(ptr_t dst, ptr_t src, size_t sz);

#endif

#ifndef MIMIK_MEM_H
#define MIMIK_MEM_H

#include <types.h>

void mem_set(void* p, uint8_t c, size_t s);
void mem_copy(void* dst, void* src, size_t sz);
void mem_copy_reverse(void* dst, void* src, size_t sz);

#endif

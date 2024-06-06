#ifndef MIMIK_MM_H
#define MIMIK_MM_H

#include <types.h>
#include <mm/vm.h>

void mm_init(void);
ptr_t mm_alloc(size_t sz);
void mm_free(ptr_t* alloc);
ptr_t mm_map(addr_t hint, size_t size, enum vm_map_flags flags);

#endif

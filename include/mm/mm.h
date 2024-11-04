#ifndef MIMIK_MM_H
#define MIMIK_MM_H

#include <types.h>
#include <mm/vm.h>

void mm_init(void);
ptr_t mm_alloc(size_t sz);
void mm_free(ptr_t* alloc);
status_t mm_map(addr_t vaddr, size_t size, flags_t flags);
status_t mm_map_in(struct pt* table, addr_t vaddr, size_t size, flags_t flags);

#endif

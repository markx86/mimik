#ifndef MIMIK_MM_H
#define MIMIK_MM_H

#include <types.h>
#include <mm/vm.h>

void mm_init(void);
ptr_t mm_alloc(size_t sz);
void mm_free(ptr_t* alloc);
ptr_t mm_map(addr_t hint, size_t size, int flags);
ptr_t mm_map_in_table(ptr_t table, addr_t hint, size_t size, int flags);

#endif

#ifndef MIMIK_PM_H
#define MIMIK_PM_H

#include <types.h>

#define ENOPAGES 0
#define EINVPAGE 1

void pm_init(addr_t free_mem_ptr);

addr_t pm_request_page(void);
addr_t pm_request_pages(size_t num);
addr_t pm_request_bytes(size_t bytes);

bool_t pm_try_lock_page(addr_t paddr);
bool_t pm_try_lock_pages(addr_t paddr_start, size_t bytes);
bool_t pm_try_lock_pages_range(addr_t paddr_start, addr_t paddr_end);

bool_t pm_try_release_page(addr_t paddr);
bool_t pm_try_release_pages(addr_t paddr_start, size_t bytes);
bool_t pm_try_release_pages_range(addr_t paddr_start, addr_t paddr_end);

#endif

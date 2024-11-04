#ifndef MIMIK_PM_H
#define MIMIK_PM_H

#include <boot/bootinfo.h>
#include <mem/page.h>
#include <types.h>

void pm_init(void);

addr_t pm_request_page(void);

void pm_lock_pages(addr_t paddr_start, size_t n);
static inline void
pm_lock_page(addr_t paddr) {
  pm_lock_pages(paddr, 1);
}
static inline void
pm_lock_range(addr_t paddr_start, addr_t paddr_end) {
  pm_lock_pages(paddr_start, paddr_end - paddr_start);
}

void pm_release_pages(addr_t paddr_start, size_t n);
static inline void
pm_release_page(addr_t paddr) {
  pm_release_pages(paddr, 1);
}
static inline void
pm_release_range(addr_t paddr_start, addr_t paddr_end) {
  pm_release_pages(paddr_start, paddr_end - paddr_start);
}

// void pm_track_page(addr_t paddr);

size_t pm_get_total_memory(void);

#endif

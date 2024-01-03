#ifndef MIMIK_PAGE_H
#define MIMIK_PAGE_H

#include <util/align.h>
#include <util/size.h>

#define PAGE_SIZE SIZE_4KB

#define PAGES(b) ((b + PAGE_SIZE - 1) / PAGE_SIZE)
#define BYTES(p) ((p) * PAGE_SIZE)

#define PAGEALIGNUP(addr) ALIGNUP(addr, PAGE_SIZE)
#define PAGEALIGNDOWN(addr) ALIGNDOWN(addr, PAGE_SIZE)

#endif

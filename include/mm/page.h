#ifndef MIMIK_PAGE_H
#define MIMIK_PAGE_H

#include <util/align.h>
#include <util/size.h>

#define PAGE_SIZE SIZE_4KB

#define PAGES(b) ((b + PAGE_SIZE - 1) / PAGE_SIZE)
#define BYTES(p) ((p) * PAGE_SIZE)

#define PAGEALIGN_UP(addr) ALIGN_UP(addr, PAGE_SIZE)
#define PAGEALIGN_DOWN(addr) ALIGN_DOWN(addr, PAGE_SIZE)

#endif

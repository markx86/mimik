#ifndef MIMIK_PAGE_H
#define MIMIK_PAGE_H

#include <util/align.h>
#include <util/size.h>

#define PAGE_SIZE SIZE_4KB

#define PAGES(b) ((b + PAGE_SIZE - 1) / PAGE_SIZE)
#define BYTES(p) ((p) * PAGE_SIZE)

#define PAGEALIGNUP(a)   ALIGNUP(a, PAGE_SIZE)
#define PAGEALIGNDOWN(a) ALIGNDOWN(a, PAGE_SIZE)

#define PAGEOFFSET(a) ((a) & (PAGE_SIZE - 1))

#define ISPAGEALIGNED(a) (PAGEOFFSET(a) == 0)

#endif

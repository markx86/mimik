#include <util/mem.h>

void mem_set(void* p, uint8_t c, size_t s) {
	uint8_t* b = p;
	while (s-- > 0)
		*(b++) = c;
}

void mem_copy(void* dst, void* src, size_t sz) {
	uint8_t *bdst = dst, *bsrc = src;
	while (sz-- > 0)
		*(bdst++) = *(bsrc++);
}

void mem_copy_reverse(void* dst, void* src, size_t sz) {
	uint8_t *bdst = dst, *bsrc = src;
	while (sz-- > 0)
		bdst[sz-1] = bsrc[sz-1];
}

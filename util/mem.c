#include <util/mem.h>

void mem_set(void* p, uint8_t c, size_t s) {
	uint8_t* b = p;
	while (s-- > 0) {
		*(b++) = c;
	}
}

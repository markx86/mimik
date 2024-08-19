#include <assert.h>

/* TODO: make this random */
uint64_t __stack_chk_guard = 0xdeadbeefdefec8ed;

void __stack_chk_fail(void) {
  ASSERT(0 && "stack smashing detected!");
}
 

#include <mem/mem.h>
#include <assert.h>

void
mem_set(ptr_t p, uint8_t c, size_t s) {
  uint8_t* b;
  size_t s_big, c_big, *big = p;
  ASSERT(p != NULL);

  /*
   * NOTE: should we check for a minimum size before thinking
   *       about doing block copies?
   */

  c_big = 0;
  for (s_big = 0; s_big < sizeof(*big); ++s_big)
    c_big = (c_big << 8) | (size_t)c;

  s_big = s / sizeof(*big);
  ASSERT(s_big <= s);
  s -= s_big * sizeof(*big);
  while (s_big-- > 0)
    *(big++) = c_big;

  b = (uint8_t*)big;
  while (s-- > 0)
    *(b++) = c;
}

void
mem_copy(ptr_t dst, ptr_t src, size_t sz) {
  uint8_t *bdst, *bsrc;
  size_t sz_big, *big_dst = dst, *big_src = src;

  ASSERT(dst != NULL);
  ASSERT(src != NULL);

  sz_big = sz / sizeof(size_t);
  ASSERT(sz_big <= sz);
  sz -= sz_big;

  while (sz_big-- > 0)
    *(big_dst++) = *(big_src++);

  bdst = (uint8_t*)big_dst;
  bsrc = (uint8_t*)big_src;
  while (sz-- > 0)
    *(bdst++) = *(bsrc++);
}

/* TODO: implement fast copy for mem_copy_reverse() */
void
mem_copy_reverse(ptr_t dst, ptr_t src, size_t sz) {
  uint8_t *bdst = dst, *bsrc = src;
  ASSERT(bdst != NULL);
  ASSERT(bsrc != NULL);
  while (sz-- > 0)
    bdst[sz - 1] = bsrc[sz - 1];
}

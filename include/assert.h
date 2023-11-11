#ifndef MIMIK_ASSERT_H
#define MIMIK_ASSERT_H

void _assert(void);

#define ASSERT(x) \
  if (!(x))       \
    _assert()

#endif

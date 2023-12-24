#ifndef MIMIK_ASSERT_H
#define MIMIK_ASSERT_H

#include <types.h>

void _assert(const char* file, size_t line, const char* msg);

#define ASSERT(x) \
  if (!(x))       \
  _assert(__FILE__, __LINE__, NULL)

#endif

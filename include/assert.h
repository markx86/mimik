#ifndef MIMIK_ASSERT_H
#define MIMIK_ASSERT_H

#include <types.h>
#include <util/compiler.h>

void _assert(const char* file, size_t line, const char* msg);

#define ASSERT(x)                      \
  do {                                 \
    if (unlikely(!(x)))                \
      _assert(__FILE__, __LINE__, #x); \
  } while (0)

#endif

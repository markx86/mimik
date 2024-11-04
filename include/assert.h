#ifndef MIMIK_ASSERT_H
#define MIMIK_ASSERT_H

#include <types.h>
#include <util/compiler.h>

void ATTRIBUTES(noreturn)
    _assert_fail(const char* file, size_t line, const char* msg);

#define _ASSERTFAIL(msg) _assert_fail(__FILE__, __LINE__, msg)

#define ASSERT(x)       \
  do {                  \
    if (UNLIKELY(!(x))) \
      _ASSERTFAIL(#x);  \
  } while (0)

#define STATICASSERT(x, m) _Static_assert(x, m)

#define TODO(msg)     _ASSERTFAIL("unimplemented " msg)
#define UNREACHABLE() _ASSERTFAIL("unreachable")

#endif

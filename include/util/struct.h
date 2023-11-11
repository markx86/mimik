#ifndef MIMIK_STRUCT_H
#define MIMIK_STRUCT_H

#include <types.h>

#define typeof __typeof__
#define offsetof(type, member) ((size_t) & ((type*)NULL)->member)
#define containerof(ptr, type, member) \
  ((type*)(((char*)ptr) - offsetof(type, member)))

#endif

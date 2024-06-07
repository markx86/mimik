#ifndef MIMIK_STRUCT_H
#define MIMIK_STRUCT_H

#include <types.h>

#define ARRAYLEN(a)            (sizeof(a) / sizeof(*a))
#define TYPEOF(elem)           __typeof__((elem))
#define OFFSETOF(type, member) ((size_t) & ((type*)NULL)->member)
#define CONTAINEROF(ptr, type, member) \
  ((type*)(((char*)ptr) - OFFSETOF(type, member)))

#endif

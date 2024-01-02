#ifndef MIMIK_ATTRIBUTES_H
#define MIMIK_ATTRIBUTES_H

#define attributes(x) __attribute__((x))

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#define PACKED attributes(packed)
#define ALIGNED(s) attributes(aligned(s))

#endif

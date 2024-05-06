#ifndef MIMIK_ATTRIBUTES_H
#define MIMIK_ATTRIBUTES_H

#define ASM(...) __asm__(__VA_ARGS__)

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define VA_LIST           __builtin_va_list
#define VA_START(ap, arg) __builtin_va_start(ap, arg)
#define VA_END(ap)        __builtin_va_end(ap)
#define VA_ARG(ap, t)     __builtin_va_arg(ap, t)

#define ATTRIBUTES(x) __attribute__((x))

#define PACKED ATTRIBUTES(packed)
#define ALIGNED(s) ATTRIBUTES(aligned(s))

#endif

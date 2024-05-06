#include <log/printk.h>
#include <log/put.h>
#include <util/compiler.h>
#include <mem/mem.h>
#include <assert.h>
#include <types.h>
#include <kernel.h>

#define BUF_SIZE 64

struct print_flags {
  size_t number_size;
  size_t leading_zeros;
  size_t string_length;
};

static void
print_unsigned(size_t zeros, uint64_t n) {
  size_t i;
  char cs[BUF_SIZE];
  ASSERT(zeros <= sizeof(cs));
  for (i = 0; n > 0; n /= 10, ++i) {
    ASSERT(i < sizeof(cs));
    cs[i] = (char)(n % 10) + '0';
  }
  for (size_t j = i; j < zeros; ++j)
    putc('0');
  for (; i > 0; --i)
    putc(cs[i - 1]);
}

static void
print_decimal(size_t zeros, int64_t n) {
  if (n < 0) {
    n = -n;
    putc('-');
  }
  print_unsigned(zeros, (uint64_t)n);
}

static void
print_hex(size_t zeros, uint64_t n, bool_t cap) {
  size_t i;
  char cs[BUF_SIZE];
  char base = cap ? 'A' : 'a';
  for (i = 0; n > 0; n >>= 4, ++i) {
    uint8_t d = n & 0xf;
    ASSERT(i < sizeof(cs));
    if (d < 10)
      cs[i] = (char)(d + '0');
    else
      cs[i] = (char)(d - 10) + base;
  }
  for (size_t j = i; j < zeros; ++j)
    putc('0');
  for (; i > 0; --i)
    putc(cs[i - 1]);
}

static void
print_ptr(ptr_t ptr, bool_t cap) {
  if (ptr == NULL) {
    puts("(nil)");
    return;
  }
  puts("0x");
  print_hex(sizeof(ptr_t) << 1, (addr_t)ptr, cap);
}

static void
print_binary(size_t sz, uint64_t n) {
  uint64_t mask;
  sz <<= 3;
  mask = 1 << (sz - 1);
  for (; sz > 0; --sz, mask >>= 1)
    putc(((mask & n) != 0) + '0');
}

static void
print_string(size_t length, const char* str) {
  if (length == 0) {
    puts(str);
    return;
  }
  for (; length > 0; --length)
    putc(*(str++));
}

void
printk(const char* fmt, ...) {
  VA_LIST ap;
  struct print_flags f = {0};

#define next_int(t) \
  (f.number_size > sizeof(int) ? VA_ARG(ap, t##64_t) : VA_ARG(ap, t##32_t))

  VA_START(ap, fmt);

  for (; *fmt != '\0'; ++fmt) {
    if (*fmt != '%') {
      putc(*fmt);
      continue;
    }

    f.number_size = sizeof(int);
    f.leading_zeros = 0;
    f.string_length = 0;

  next_mod:
    switch (*(++fmt)) {
      case '%':
        putc('%');
        break;
      case 'l':
        if (f.number_size < sizeof(size_t))
          f.number_size <<= 1;
        goto next_mod;
      case 'h':
        if (f.number_size > 1)
          f.number_size >>= 1;
        goto next_mod;
      case '0':
        for (++fmt; *fmt >= '0' && *fmt <= '9'; ++fmt) {
          f.leading_zeros *= 10;
          f.leading_zeros += (size_t)(*fmt - '0');
        }
        --fmt;
        goto next_mod;
      case 'b':
        print_binary(
            f.leading_zeros != 0 ? f.leading_zeros : f.number_size,
            next_int(uint));
        break;
      case 'd':
        print_decimal(f.leading_zeros, next_int(int));
        break;
      case 'u':
        print_unsigned(f.leading_zeros, next_int(uint));
        break;
      case 'x':
      case 'X':
        print_hex(f.leading_zeros, next_int(uint), *fmt == 'X');
        break;
      case 'p':
      case 'P':
        print_ptr(VA_ARG(ap, ptr_t), *fmt == 'P');
        break;
      case 's':
        print_string(f.string_length, VA_ARG(ap, const char*));
        break;
      case 'c':
        putc((char)VA_ARG(ap, int));
        break;
      default:
        if (*fmt >= '0' && *fmt <= '9') {
          f.string_length *= 10;
          f.string_length += (size_t)(*fmt - '0');
          goto next_mod;
        }
        break;
    }
  }

  VA_END(ap);
}

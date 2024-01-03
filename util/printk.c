#include <util/printk.h>
#include <util/put.h>
#include <util/compiler.h>
#include <util/mem.h>
#include <assert.h>
#include <types.h>
#include <kernel.h>

#define BUF_SIZE 64

struct print_flags {
  size_t number_size;
  size_t leading_zeros;
};

static void
print_unsigned(size_t zeros, uint64_t n) {
  size_t i;
  char cs[BUF_SIZE];
  ASSERT(zeros <= sizeof(cs));
  for (i = 0; n > 0; n /= 10, ++i) {
    ASSERT(i < sizeof(cs));
    cs[i] = n % 10 + '0';
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
  print_unsigned(zeros, n);
}

static void
print_hex(size_t zeros, uint64_t n, bool_t cap) {
  size_t i;
  char cs[BUF_SIZE];
  char base = cap ? 'A' : 'a';
  for (i = 0; n > 0; n >>= 4, ++i) {
    ASSERT(i < sizeof(cs));
    uint8_t d = n & 0xF;
    if (d < 10)
      cs[i] = d + '0';
    else
      cs[i] = (d - 10) + base;
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

void
printk(const char* fmt, ...) {
  va_list ap;
  struct print_flags f = {0};

#define next_int(t) \
  (f.number_size > sizeof(int) ? va_arg(ap, t##64_t) : va_arg(ap, t##32_t))

  va_start(ap, fmt);

  for (; *fmt != '\0'; ++fmt) {
    if (*fmt != '%') {
      putc(*fmt);
      continue;
    }

    f.number_size = sizeof(int);
    f.leading_zeros = 0;

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
          f.leading_zeros += *fmt - '0';
        }
        --fmt;
        goto next_mod;
      case 'b':
        print_binary(f.number_size, next_int(uint));
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
        print_ptr(va_arg(ap, ptr_t), *fmt == 'P');
        break;
      case 's':
        puts(va_arg(ap, const char*));
        break;
      case 'c':
        putc((char)va_arg(ap, int));
        break;
      default:
        break;
    }
  }

  va_end(ap);
}

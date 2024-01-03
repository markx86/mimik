#ifndef MIMIK_PUT_H
#define MIMIK_PUT_H

void putc(char c);

static inline void
puts(const char* s) {
  while (*s != '\0')
    putc(*(s++));
}

#endif

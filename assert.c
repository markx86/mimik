#include <assert.h>
#include <log/log.h>

void
_assert(const char* file, size_t line, const char* expr) {
  const char *cur, *root, *this_file = __FILE__;

  /* TODO: replace this with something like strchrr, maybe? */
  cur = this_file;
  while (*cur != '\0') {
    if (*cur == '/')
      root = cur + 1;
    ++cur;
  }
  file += (size_t)(root - this_file);

  printk(
    "[" LOG_COLOR_FATAL "%s:%lu" LOG_COLOR_RESET "] Assertion failed: %s\n",
    file, line, expr);

  while (TRUE)
    __asm__("hlt");
}

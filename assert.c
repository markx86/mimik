#include <assert.h>
#include <log/printk.h>

void
_assert(const char* file, size_t line, const char* expr) {
  const char* last_path_sep = file;

  /* TODO: replace this with something like strchrr, maybe? */
  while (*file != '\0') {
    if (*file == '/')
      last_path_sep = file;
    ++file;
  }

  printk("[%s:%lu] Assertion failed: %s\n", last_path_sep + 1, line, expr);

  while (TRUE)
    __asm__("hlt");
}

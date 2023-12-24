#include <assert.h>
#include <util/printk.h>

void
_assert(const char* file, size_t line, const char* msg) {
  char line_num[16];
  size_t counter;
  const char* last_path_sep = file;

  /* TODO: replace this with something like strchrr, maybe? */
  while (*file != '\0') {
    if (*file == '/')
      last_path_sep = file;
    ++file;
  }

  /* TODO: replace this with something like itoa, maybe? */
  for (counter = 0; line > 0 && counter < sizeof(line_num) - 1; ++counter) {
    size_t num = line % 10;
    line_num[counter] = '0' + num;
    line /= 10;
  }
  line_num[counter] = '\0';

  /* TODO: what the fuck is this, do it properly ewww */
  printk("[ ");
  printk(last_path_sep + 1);
  printk(" @ ");
  printk(line_num);
  printk(" ] Assertion failed: ");
  printk(msg);
  printk("\n");

  while (TRUE)
    __asm__("hlt");
}

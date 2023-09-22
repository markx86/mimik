#ifdef MIMIK_BOOTPROTOCOL_MULTIBOOT2

#define ASM_FILE
#include "multiboot2.h"

.section .multiboot.data , "a", @progbits
/* Multiboot header. */
#define MULTIBOOT2_HEADER_LENGTH (multiboot2_header_end-multiboot2_header)
#define MULTIBOOT2_HEADER_CHECKSUM  \
  -(MULTIBOOT2_HEADER_MAGIC +       \
    MULTIBOOT2_ARCHITECTURE_I386 +  \
    MULTIBOOT2_HEADER_LENGTH)
multiboot2_header:
/* multiboot2 header fields */
  .align MULTIBOOT_HEADER_ALIGN
  .long MULTIBOOT2_HEADER_MAGIC
  .long MULTIBOOT2_ARCHITECTURE_I386
  .long MULTIBOOT2_HEADER_LENGTH
  .long MULTIBOOT2_HEADER_CHECKSUM
/* module alignment tag */
module_alignment:
  .short MULTIBOOT_HEADER_TAG_MODULE_ALIGN
  .short 0
  .long (console_flags-module_alignment)
/* console flags tag */
console_flags:
  .short MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS
  .short MULTIBOOT_HEADER_TAG_OPTIONAL
  .long (framebuffer-console_flags)
  .long MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED
/* framebuffer tag */
framebuffer:
  .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
  .short MULTIBOOT_HEADER_TAG_OPTIONAL
  .long (ending-framebuffer)
  .long 0
  .long 0
  .long 0
/* end tag */
ending:
  .short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long (multiboot2_header_end-ending)
multiboot2_header_end:

#endif /* MIMIK_BOOTPROTOCOL_MULTIBOOT2 */

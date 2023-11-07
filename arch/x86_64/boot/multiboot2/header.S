#define ASM_FILE
#include "multiboot2.h"

.section .multiboot.data , "aw", @progbits
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
  .long (ending-module_alignment)
/* end tag */
ending:
  .short MULTIBOOT_HEADER_TAG_END
  .short 0
  .long (multiboot2_header_end-ending)
multiboot2_header_end:

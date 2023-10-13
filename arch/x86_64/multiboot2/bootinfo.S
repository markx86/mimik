#ifdef MIMIK_BOOTPROTOCOL_MULTIBOOT2

#define ASM_FILE
#include "multiboot2.h"

.section .multiboot.data, "aw", @progbits
.align 8
.global _bootinfo
_bootinfo:
  cmdline: .quad 0
  modules: .quad 0
  memmap_entries: .quad 0
  memmap_segments: .quad 0
  acpi_type: .quad 0
  acpi_struct: .fill 40, 1, 0

.section .multiboot.text, "ax"
.code32

.global init_bootinfo
init_bootinfo:
  /* check the bootloader signature */
  cmpl $MULTIBOOT2_BOOTLOADER_MAGIC, %eax
  jne _fail
  /* parse the multiboot info struct */
  pushl %ebx
  movl %ebx, %edi
  movl (%edi), %esi
  addl $8, %edi
  movl $_bootinfo_data, %ebx
  /*
   * (esp): the pointer to the start of the multiboot info struct
   * edi: the pointer to the start of current tag of the multiboot info struct
   * esi: the size of the multiboot info struct
   * ebx: the pointer to the bootinfo data space
   */
parse_next_tag:
  movl %edi, %edx
  movl (%edx), %eax
  addl $4, %edx
  movl (%edx), %ecx
  addl $4, %edx
  subl $8, %ecx
  /*
   * edx: the pointer to a field of the current multiboot tag
   * eax: the type of the current multiboot tag
   * ecx: the size of the current multiboot tag (excluding type, size fields)
   */

  /* check if we reached the last tag */
  cmpl $MULTIBOOT_TAG_TYPE_END, %eax
  je end
  /* otherwise check if we need to handle the tag */
  
  cmpl $MULTIBOOT_TAG_TYPE_CMDLINE, %eax
  jne 1f
  /* handle cmdline tag */
  /* ecx: the length of the string */
  /* copy cmdline string */
  movl %ebx, (cmdline)
0:
  movb (%edx), %al
  incl %edx
  movb %al, (%ebx)
  incl %ebx
  loop 0b
1:

  cmpl $MULTIBOOT_TAG_TYPE_MODULE, %eax
  jne 4f
  /* handle module tag */
  /* check if we already found one module */
  movl (modules), %eax
  cmpl $0, %eax
  jne 0f
  /* if we didn't initialize bootinfo.modules */
  movl %ebx, (modules)
  movl %ebx, %eax
  jmp 2f
0:
  /*
   * if we already found at least one module,
   * find the last module in the list and insert the pointer to a new module
   */
  cmpl $0, (%eax)
  je 1f
  movl (%eax), %eax
  jmp 0b
1:
  movl %ebx, (%eax)
2:
  /* fill the bootmodule struct */
  /* bootinfo_module.next (initialize to NULL) */
  /*
   * NOTE: since we're writing 64 bit data using 32 bit registers on a LSB
   *       machine, we have to write the lower 32 bits first and the
   *       upper 32 bits second. This is a recurring thing in this function.
   */
  /* write lower 32 bits */
  movl $0, (%ebx)
  addl $4, %ebx
  /* write upper 32 bits */
  movl $0, (%ebx)
  addl $4, %ebx
  /* bootinfo_module.start_address */
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  movl $0, (%ebx)
  addl $4, %ebx
  /* bootinfo_module.end_address */
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  movl $0, (%ebx)
  addl $4, %ebx
  /* bootinfo_module.cmdline */
  subl $8, %ecx
3:
  movb (%edx), %al
  incl %edx
  movb %al, (%ebx)
  incl %ebx
  loop 3b
4:

  cmpl $MULTIBOOT_TAG_TYPE_MMAP, %eax
  jne 3f
  /* handle memory map tag */
  /* compute the number of entries in the memory map */
  movl %ecx, %eax
  pushl %ebx
  movl (%edx), %ebx
  pushl %edx
  xorl %edx, %edx
  divl %ebx
  popl %edx
  popl %ebx
  addl $8, %edx
  movl %eax, %ecx
  movl %ecx, (memmap_entries)
  /* initialize pointer to memory map entries */
  movl %ebx, (memmap_segments)
  /*
   * loop variables
   * (esp): the size of each entry
   * ecx: number of entries left
   * ebx: pointer to a member of the bootinfo_memsegment struct
   * edx: pointer to a member of the multiboot_mmap_entry struct
   */
0:
  /* check if the memory map entry is for usable memory */
  movl %edx, %eax
  addl $16, %eax
  /* MULTIBOOT_MEMORY_AVAILABLE = 1 */
  cmpl $1, (%eax)
  jne 1f
  /*
   * if it is, set the type of the segment
   * to BOOTINFO_MEMSEGMENT_TYPE_AVAILABLE
   */
  movl $1, (%ebx)
  addl $4, %ebx
  movl $0, (%ebx)
  jmp 2f
1:
  /*
   * if it is not, set the type of the segment
   * to BOOTINFO_MEMSEGMENT_TYPE_RESERVED
   */
  movl $0, (%ebx)
  addl $4, %ebx
  movl $0, (%ebx)
2:
  addl $4, %ebx
  /* save bootinfo_memsegment.addr */
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  /* save bootinfo_memsegment.length */
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  movl (%edx), %eax
  addl $4, %edx
  movl %eax, (%ebx)
  addl $4, %ebx
  addl $8, %edx
  loop 0b
3:

  cmpl $MULTIBOOT_TAG_TYPE_FRAMEBUFFER, %eax
  jne 0f
  /* TODO: handle framebuffer tag */
0:

  cmpl $MULTIBOOT_TAG_TYPE_ACPI_OLD, %eax
  jne 0f
  /* handle rsdp */
  movl $1, (acpi_type)
  jmp 1f
0:
  cmpl $MULTIBOOT_TAG_TYPE_ACPI_NEW, %eax
  jne 3f
  /* handle xsdp */
  movl $2, (acpi_type)
1:
  /* copy the acpi struct */
  pushl %ebx
  movl $acpi_struct, %ebx
2:
  movb (%edx), %al
  incl %edx
  movb %al, (%ebx)
  incl %ebx
  loop 2b
  popl %ebx
3:

  /* advance to next tag */
  /* 1. compute the tag end address */
  addl $4, %edi
  addl (%edi), %edi
  subl $4, %edi
  /* 2. align the tag end address to 8 bytes */
  addl $7, %edi
  andl $~7, %edi
  /* 4. align the bootinfo data pointer to 8 bytes */
  addl $7, %ebx
  andl $~7, %ebx
  /* check we reached the end of the multiboot info struct */
  movl %edi, %eax
  subl (%esp), %eax
  cmpl %esi, %eax
  jge end
  jmp parse_next_tag
end:
  /* clear stack (remember that (esp) holds the size of the multiboot struct) */
  popl %eax
  movl %ebx, %eax
  ret

#endif
#include <cpu/gdt.h>

.section .rodata, "a", @progbits

gdt:
  /* define null segment */
  GDTENTRY(
    null,
    0,
    0,
    0,
    0)
  /*
   * define kernel code segment with the following properties
   * access: is present, privilege level 0, not a system segment,
   *         executable and readable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDTENTRY(
    kernel_code,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(0) +
    ACCESS_NORMALSEGMENT +
    ACCESS_EXECUTABLE +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE + 
		FLAGS_LONGMODE)
  /*
   * define kernel data segment with the following properties
   * access: is present, privilege level 0, not a system segment,
   *         not executable and writable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDTENTRY(
    kernel_data,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(0) +
    ACCESS_NORMALSEGMENT +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE +
		FLAGS_LONGMODE)
  /*
   * define kernel user segment with the following properties
   * access: is present, privilege level 3, not a system segment,
   *         executable and readable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDTENTRY(
    user_code,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(3) +
    ACCESS_NORMALSEGMENT +
    ACCESS_EXECUTABLE +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE +
		FLAGS_LONGMODE)
  /*
   * define user data segment with the following properties
   * access: is present, privilege level 3, not a system segment,
   *         not executable and writable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDTENTRY(
    user_data,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(3) +
    ACCESS_NORMALSEGMENT +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE +
		FLAGS_LONGMODE)
gdt_end:

gdtr:
  .short (gdt_end-gdt)-1
  .quad gdt

.section .text, "ax"
.code64
.global gdt_load
gdt_load:
  lgdtq (gdtr)
  /*
   * retfq expects the following data on the stack:
   * (rsp+0): the return address
   * (rsp+8): the gdt code segment selector
   */
  popq %rax
  pushq $KERNEL_CS
  pushq %rax
  retfq

#include <cpu/gdt.h>

.section .entry.data, "aw", @progbits

boot_gdt:
  /* define null segment */
  GDTENTRY(
    boot_null,
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
    boot_code,
    0x0000000000000000,
    0xfffff,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(0) +
    ACCESS_NORMALSEGMENT +
    ACCESS_EXECUTABLE +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE)
  /*
   * define kernel data segment with the following properties
   * access: is present, privilege level 0, not a system segment,
   *         not executable and writable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDTENTRY(
    boot_data,
    0x0000000000000000,
    0xfffff,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGELEVEL(0) +
    ACCESS_NORMALSEGMENT +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE)
boot_gdt_end:

#define BOOT_CS (_gdt_boot_code-_gdt_boot_null)
#define BOOT_DS (_gdt_boot_data-_gdt_boot_null)

.align 4
boot_gdtr:
  .short (boot_gdt_end-boot_gdt)-1;
  .long boot_gdt

.section .entry.text, "ax"
.code32

/* loads the proteced mode gdtr and reloads the segment registers */
.global load_boot_gdt
load_boot_gdt:
  lgdtl (boot_gdtr)
  jmp $BOOT_CS,$reload_cs32
reload_cs32:
  movw $BOOT_DS, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %fs
  movw %ax, %ss
  ret

#define UPDATEFLAGS(label)                 \
  movl (_gdt_##label##_flags), %eax       ;\
  andl $~(FLAGS_PROTECTEDMODE << 4), %eax ;\
  orl $(FLAGS_LONGMODE << 4), %eax        ;\
  movl %eax, (_gdt_##label##_flags)

/* updates the gdt and switches to long mode */
.global edit_boot_gdt
edit_boot_gdt:
  /* update the gdt to work with long mode addressing */
  UPDATEFLAGS(boot_code)
  UPDATEFLAGS(boot_data)
  /* flush the instruction cache and jump into 64 bit long mode code */
  jmp $BOOT_CS,$far_return
.code64
far_return:
  /*
   * NOTE: there's no need to reload the segment register's
   * since the gdt remains the same.
   * 
   * we need to do a far return to reload the gdt
   * lretq expects the following data on the stack:
   * (rsp+0): the return address
   * (rsp+8): the gdt code segment selector
   * in this case we want to return to our 64 bit long mode entry in the higher
   * half (_stub64) and with cs pointing to the kernel code segment
   */
  addq $4, %rsp
  pushq $BOOT_CS
  pushq $_stub64
  lretq

.section .entry.data, "aw", @progbits

#define GDT_ENTRY(label, base, limit, access, flags) \
gdt_##label:;                                        \
  .short (limit) & 0xFFFF;                           \
  .short (base) & 0xFFFF;                            \
  .byte ((base) >> 16) & 0xFF;                       \
  .byte (access);                                    \
gdt_##label##_flags:;                                \
  .byte ((flags) << 4) | (((limit) >> 16) & 0xF);    \
  .byte ((base) >> 24) & 0xFF
#define GDT_SYSENTRY(label, base, limit, access, flags) \
  GDT_ENTRY(label, base, limit, access, flags);         \
  .long (base) >> 32;                                   \
  .long 0

#define ACCESS_PRESENT (1 << 7)
#define ACCESS_PRIVILEGE_LEVEL(l) (((l) & 0b11) << 5)
#define ACCESS_NORMAL_SEGMENT (1 << 4)
#define ACCESS_EXECUTABLE (1 << 3)
#define ACCESS_READWRITE (1 << 1)

#define FLAGS_GRANULARITY (1 << 3)
#define FLAGS_PROTECTEDMODE (1 << 2)
#define FLAGS_LONGMODE (1 << 1)

gdt:
  /* define null segment */
  GDT_ENTRY(
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
  GDT_ENTRY(
    kernel_code,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGE_LEVEL(0) +
    ACCESS_NORMAL_SEGMENT +
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
  GDT_ENTRY(
    kernel_data,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGE_LEVEL(0) +
    ACCESS_NORMAL_SEGMENT +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE)
  /*
   * define kernel user segment with the following properties
   * access: is present, privilege level 3, not a system segment,
   *         executable and readable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDT_ENTRY(
    user_code,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGE_LEVEL(3) +
    ACCESS_NORMAL_SEGMENT +
    ACCESS_EXECUTABLE +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE)
  /*
   * define user data segment with the following properties
   * access: is present, privilege level 3, not a system segment,
   *         not executable and writable
   * flags: has 4kB granularity, is a protected mode segment
   */
  GDT_ENTRY(
    user_data,
    0x0000000000000000,
    0xFFFFF,
    ACCESS_PRESENT +
    ACCESS_PRIVILEGE_LEVEL(3) +
    ACCESS_NORMAL_SEGMENT +
    ACCESS_READWRITE,
    FLAGS_GRANULARITY +
    FLAGS_PROTECTEDMODE)
gdt_end:

#define GDT_KERNEL_CS (gdt_kernel_code-gdt)
#define GDT_KERNEL_DS (gdt_kernel_data-gdt)
#define GDT_USER_CS (gdt_user_code-gdt)
#define GDT_USER_DS (gdt_user_data-gdt)

/* gdtr for both protected mode and long mode */
.align 4
gdtr32:
  .short (gdt_end-gdt)-1;
  .long gdt

.section .entry.text, "ax"
.code32

/* loads the proteced mode gdtr and reloads the segment registers */
.global load_gdt
load_gdt:
  lgdtl (gdtr32)
  jmp $GDT_KERNEL_CS,$reload32

reload32:
  movw $GDT_KERNEL_DS, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %fs
  movw %ax, %ss
  ret

#define update_flags(label)         \
  movl (gdt_##label##_flags), %eax; \
  orl $(FLAGS_LONGMODE << 4), %eax; \
  movl %eax, (gdt_##label##_flags)

/* updates the gdt to work with long mode */
.global edit_gdt
edit_gdt:
  update_flags(kernel_code)
  update_flags(kernel_data)
  update_flags(user_code)
  update_flags(user_data)
  /* flush the instruction cache and jump into 64 bit long mode code */
  jmp $GDT_KERNEL_CS,$reload64

.code64
reload64:
  /* we reload the gdt to work with long mode addressing */
  lgdtq (gdtr64)
  /*
   * NOTE: there's no need to reload the segment register's
   * since the gdt remains the same.
   * 
   * we need to do a far return to reload the gdt
   * retfq expects the following data on the stack:
   * (rsp+0): the return address
   * (rsp+8): the gdt code segment selector
   * in this case we want to return to our 64 bit long mode entry in the higher
   * half (_stub64) and with cs pointing to the kernel code segment
   */
  addq $4, %rsp
  pushq $GDT_KERNEL_CS
  movq $_stub64, %rax
  pushq %rax
  retfq

.section .rodata, "a", @progbits
gdtr64:
  .short (gdt_end-gdt)-1
  .quad (gdt+HIGHER_HALF)

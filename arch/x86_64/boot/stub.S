.section .entry.bss, "aw", @nobits
.align 8
stack_bottom:
  .skip 4096
stack_top:
#define STACK (stack_top-8)

#define BOOTINFOENTRY(name, asm) \
  .global name;                   \
  name: asm
#define C ,
.section .entry.data, "aw", @progbits
.align 8
_bootinfo:
  BOOTINFOENTRY(_bootinfo_cmdline, .quad 0)
  BOOTINFOENTRY(_bootinfo_modules, .quad 0)
  BOOTINFOENTRY(_bootinfo_mem_map_entries, .quad 0)
  BOOTINFOENTRY(_bootinfo_mem_map_segments, .quad 0)
  BOOTINFOENTRY(_bootinfo_acpi_type, .quad 0)
  BOOTINFOENTRY(_bootinfo_acpi_struct, .fill 40 C 1 C 0)
#undef C

.section .entry.text, "ax"
.code32

/**
 * _stub32: entry for protected mode bootprotocols
 *
 * Expected machine state:
 * - rax: pointer to bootinfo data end address
 */
.global _stub32
_stub32:
  /* disable interrupts */
  cli
  /* setup stack */
  movl $STACK, %esp
  /* set the last return address to a stub function */
  movl $_fail, (%esp)
  /* store the bootinfo data end address */
  pushl $0
  pushl %eax
  /* copy the bootinfo data end address as it's needed in setup_paging */
  pushl %eax
  /* load GDT */
  call load_boot_gdt
  /* check if the required features are supported */
  call check_feature_support
  /* setup paging */
  popl %ebx
  call setup_paging
  /* enable features and long mode */
  call enable_features
  /* edit gdt */
  call edit_boot_gdt
  jmp _fail

.global _fail
_fail:
  hlt
  jmp _fail

#define EFLAGS_BIT_ID (1 << 21)

#define CPUID_EXTFN_SUPPORT 0x80000000
#define CPUID_EXTFN_FEATURES 0x80000001

#define CPUID_FEATURES_MSR (1 << 5)
#define CPUID_FEATURES_PAE (1 << 6)
#define CPUID_FEATURES_SYSCALL (1 << 11)
#define CPUID_FEATURES_LM (1 << 29)
#define CPUID_FEATURES_NX (1 << 20)

check_feature_support:
  /* 1. check if cpuid is supported */
  /* store the content of the EFLAGS register in eax */
  pushfl
  popl %eax
  /* save a copy in ecx */
  movl %eax, %ecx
  /* flip the cpuid bit */
  xorl $EFLAGS_BIT_ID, %eax
  /* write the eax back to the EFLAGS register */
  pushl %eax
  popfl
  /* check if the bit remained flipped */
  pushfl
  popl %eax
  cmpl %eax, %ecx
  /* if it didn't, fail */
  jz _fail
  /* 2. check if the cpuid supports extended functions */
  movl $CPUID_EXTFN_SUPPORT, %eax
  cpuid
  cmpl $CPUID_EXTFN_SUPPORT, %eax
  jle _fail
  /* if it is, get processor features */
  movl $CPUID_EXTFN_FEATURES, %eax
  cpuid
  /* 3. check if long mode is supported */
  testl $CPUID_FEATURES_LM, %edx
  /* if it isn't, fail */
  jz _fail
  /* 4. check if PAE is supported */
  testl $CPUID_FEATURES_PAE, %edx
  jz _fail
  /* 5. check if MSRs operations are supported */
  testl $CPUID_FEATURES_MSR, %edx
  jz _fail
  /* 6. check if syscall/sysret instructions are supported */
  testl $CPUID_FEATURES_SYSCALL, %edx
  jz _fail
  /* 7. check if no-execute bit is supported */
  testl $CPUID_FEATURES_NX, %edx
  jz _fail
  ret

#define MSR_EFER 0xC0000080
#define MSR_EFER_BIT_LM (1 << 8)
#define MSR_EFER_BIT_NX (1 << 11)

#define CR0_BIT_PG (1 << 31)
#define CR4_BIT_PAE (1 << 5)

enable_features:
  /* enable PAE */
  movl %cr4, %eax
  orl $CR4_BIT_PAE, %eax
  movl %eax, %cr4
  /* enable EFER features */
  movl $MSR_EFER, %ecx
  rdmsr
  /* enable long mode */
  orl $MSR_EFER_BIT_LM, %eax
  /* enable no-execute bit */
  orl $MSR_EFER_BIT_NX, %eax
  wrmsr
  /* enable paging */
  movl %cr0, %eax
  orl $CR0_BIT_PG, %eax
  movl %eax, %cr0
  ret

.section .text, "ax"
.code64

/**
 * _stub64: entry for long mode bootprotocols
 *
 * Expected machine state:
 * - a valid 64 bit gdt with the same segments as the one defined in gdt.S
 * - the kernel mapped correctly into higher half
 * - (rsp): pointer to bootinfo data end address
 */
.global _stub64
_stub64:
  /* kernel main parameters (translated into higher half addresses) */
  movq $(_bootinfo+HIGHER_HALF), %rdi
  movq $(_bootinfo_data+HIGHER_HALF), %rsi
  popq %rdx
  addq $HIGHER_HALF, %rdx
  /* load the higher half stack */
  movq $(STACK+HIGHER_HALF), %rsp
  movq %rsp, %rbp
  /* set the fail address */
  movq $(_fail+HIGHER_HALF), (%rsp)
  /* make every pointer in _bootinfo a higher half pointer */
  addq $HIGHER_HALF, (_bootinfo+0)
  addq $HIGHER_HALF, (_bootinfo+24)
  movq (_bootinfo+8), %rax
0:
  addq $HIGHER_HALF, %rax
  movq (%rax), %rax
  cmpq $0, %rax
  jne 0b
  call kernel_main
stall:
  hlt
  jmp stall

.section .entry.bss, "aw", @nobits
.align 8
stack_bottom:
  .skip 4096
stack_top:

.section .entry.text, "ax"
.code32

.global _start32
_start32:
  /* disable interrupts */
  cli
  /* setup stack */
  movl $(stack_top-8), %esp
  /* set the last return address to a stub function */
  movl $_fail, (%esp)
  /* initialize boot information */
  call init_bootinfo
  /* store the bootinfo data end address */
  pushl $0
  pushl %eax
  /* load GDT */
  call load_gdt
  /* check if the required features are supported */
  call check_feature_support
  /* setup paging */
  call setup_paging
  /* enable features and long mode */
  call enable_features
  /* edit gdt */
  call edit_gdt
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
  ret

#define MSR_EFER 0xC0000080
#define MSR_EFER_BIT_LM (1 << 8)
#define CR0_BIT_PG (1 << 31)
#define CR4_BIT_PAE (1 << 5)

enable_features:
  /* enable PAE */
  movl %cr4, %eax
  orl $CR4_BIT_PAE, %eax
  movl %eax, %cr4
  /* enable long mode */
  movl $MSR_EFER, %ecx
  rdmsr
  orl $MSR_EFER_BIT_LM, %eax
  wrmsr
  /* enable paging */
  movl %cr0, %eax
  orl $CR0_BIT_PG, %eax
  movl %eax, %cr0
  ret

.section .text, "ax"
.code64

.global _start64
_start64:
  /* make the stack pointer point into higher half memory */
  addq $HIGHER_HALF, %rsp
  /* kernel main parameters (translated into higher half addresses) */
  movq $(_bootinfo+HIGHER_HALF), %rdi
  movq $(_bootinfo_data+HIGHER_HALF), %rsi
  popq %rdx
  addq $HIGHER_HALF, %rdx
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
